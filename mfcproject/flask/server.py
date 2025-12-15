from flask import Flask, render_template, request, jsonify, session, redirect, url_for
from flask_socketio import SocketIO, emit, join_room
import base64
import requests
from functools import wraps
import pymysql
import subprocess
import threading
from datetime import datetime, timedelta
import jwt


app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-very-secret-key-for-session-management'


app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = '1234'
app.config['MYSQL_DB'] = 'yolo_webapp_db'

socketio = SocketIO(app)


RPI_DETECT_URL = "http://192.168.0.104:18080/detect"


yolo_lock = threading.Lock()
sid_to_user = {}
active_yolo_target_username = None



def get_db_connection():
    return pymysql.connect(host=app.config['MYSQL_HOST'], user=app.config['MYSQL_USER'], password=app.config['MYSQL_PASSWORD'], db=app.config['MYSQL_DB'], charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)

def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session or 'full_name' not in session:
            session.clear()
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

def admin_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session or 'full_name' not in session or not session.get('is_admin'):
            session.clear()
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function




@app.route('/')
def home():
    if 'user_id' in session:
        return redirect(url_for('admin_page') if session.get('is_admin') else url_for('employee_page'))
    return redirect(url_for('login'))

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        login_type = request.form.get('login_type')
        conn = get_db_connection()
        try:
            with conn.cursor() as cursor:
                sql = "SELECT u.*, e.full_name, e.is_admin FROM users u JOIN employees e ON u.employee_id = e.employee_id WHERE u.username = %s"
                cursor.execute(sql, (username,))
                user = cursor.fetchone()

                if user and user['password'] == password:
                    actual_is_admin = user['is_admin'] == 1
                    selected_is_admin = login_type == 'admin'

                    if actual_is_admin != selected_is_admin:
                        return render_template('login.html', error='선택한 계정 유형이 올바르지 않습니다.')
                    
                    session['user_id'] = user['user_id']
                    session['full_name'] = user['full_name']
                    session['username'] = user['username'] 
                    session['is_admin'] = actual_is_admin
                    
                    return redirect(url_for('admin_page') if actual_is_admin else url_for('employee_page'))
                else:
                    return render_template('login.html', error='아이디 또는 비밀번호가 잘못되었습니다.')
        finally:
            conn.close()
    return render_template('login.html')

@app.route('/signup', methods=['GET', 'POST'])
def signup():
    if request.method == 'POST':
        data = request.json
        conn = get_db_connection()
        try:
            with conn.cursor() as cursor:
                cursor.execute("SELECT * FROM employees WHERE employee_number = %s", (data.get('employee_number'),))
                employee = cursor.fetchone()
                if not employee: return jsonify({'success': False, 'message': '존재하지 않는 사원 번호입니다.'})
                if employee['is_registered']: return jsonify({'success': False, 'message': '이미 등록된 사원입니다.'})
                cursor.execute("SELECT * FROM users WHERE username = %s OR email = %s", (data.get('username'), data.get('email')))
                if cursor.fetchone(): return jsonify({'success': False, 'message': '이미 사용 중인 사용자 이름 또는 이메일입니다.'})
                
                insert_sql = "INSERT INTO users (employee_id, username, password, email, phone_number, status) VALUES (%s, %s, %s, %s, %s, 'offline')"
                cursor.execute(insert_sql, (employee['employee_id'], data.get('username'), data.get('password'), data.get('email'), data.get('phone_number')))
                update_sql = "UPDATE employees SET full_name = %s, is_registered = 1 WHERE employee_id = %s"
                cursor.execute(update_sql, (data.get('full_name'), employee['employee_id']))
            conn.commit()
            return jsonify({'success': True, 'message': '등록 성공! 로그인 페이지로 이동합니다.'})
        except Exception as e:
            conn.rollback()
            return jsonify({'success': False, 'message': f'데이터베이스 오류: {e}'})
        finally:
            conn.close()
    return render_template('register.html')

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))

@app.route('/admin')
@admin_required
def admin_page():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            sql = """
                SELECT u.username, e.full_name, u.status 
                FROM users u
                JOIN employees e ON u.employee_id = e.employee_id
                ORDER BY u.username
            """
            cursor.execute(sql)
            all_users = cursor.fetchall()
    finally:
        conn.close()
    return render_template('admin.html', all_users=all_users, display_name=session.get('full_name'))


@app.route('/employee')
@login_required
def employee_page():

    try:

        payload = {
            'user_id': session['user_id'],
            'exp': datetime.utcnow() + timedelta(seconds=60)
        }

        mfc_token = jwt.encode(payload, app.config['SECRET_KEY'], algorithm='HS256')
    except Exception as e:
        mfc_token = None
        print(f"Token generation error: {e}")

    
    return render_template('employee.html', 
                           display_name=session.get('full_name'),
                           mfc_token=mfc_token)


@app.route("/api/get_statuses")
@admin_required
def get_statuses():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT username, status FROM users")
            users = cursor.fetchall()
            user_statuses = {user['username']: user['status'] for user in users}
            return jsonify(user_statuses)
    finally:
        conn.close()

@app.route("/api/update_status", methods=["POST"])
@login_required
def update_status():
    new_status = request.json.get('status')
    user_id = session.get('user_id')
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("UPDATE users SET status = %s WHERE user_id = %s", (new_status, user_id))
        conn.commit()
        return jsonify({"status": "success", "message": f"상태가 '{new_status}'로 변경되었습니다."})
    finally:
        conn.close()

@app.route("/run-mfc", methods=["POST"])
@admin_required
def run_mfc():
    try:
        mfc_file_path = r"C:\Work\2025_09_19\Code\MFCApplication1\x64\Debug\MFCApplication1.exe"
        subprocess.Popen([mfc_file_path])
        return jsonify({"success": "success", "message": "프로그램이 서버에서 시작되었습니다."})
    except FileNotFoundError:
        return jsonify({"status": "error", "message": "지정된 경로에서 프로그램을 찾을 수 없습니다."}), 404
    except Exception as e:
        return jsonify({"status": "error", "message": f"실행 중 오류 발생: {e}"}), 500

@app.route('/api/test')
def api_test():
    print("MFC 클라이언트로부터 테스트 요청을 받았습니다!")
    return jsonify({'status': 'ok', 'message': 'Flask 서버가 성공적으로 응답했습니다!'})

@app.route('/api/mfc/verify-token', methods=['POST'])
def verify_mfc_token():
    data = request.get_json()
    token = data.get('token')
    if not token:
        return jsonify({'success': False, 'message': '토큰이 없습니다.'})

    try:

        payload = jwt.decode(token, app.config['SECRET_KEY'], algorithms=['HS256'])
        user_id = payload['user_id']


        conn = get_db_connection()
        with conn.cursor() as cursor:
            sql = "SELECT u.*, e.full_name, e.is_admin FROM users u JOIN employees e ON u.employee_id = e.employee_id WHERE u.user_id = %s"
            cursor.execute(sql, (user_id,))
            user = cursor.fetchone()
        conn.close()

        if not user:
            return jsonify({'success': False, 'message': '사용자를 찾을 수 없습니다.'})


        user_info = {
            'user_id': user['user_id'],
            'username': user['username'],
            'full_name': user['full_name'],
            'is_admin': user['is_admin'] == 1
        }
        return jsonify({'success': True, 'user_info': user_info})

    except jwt.ExpiredSignatureError:
        return jsonify({'success': False, 'message': '토큰이 만료되었습니다.'})
    except jwt.InvalidTokenError:
        return jsonify({'success': False, 'message': '유효하지 않은 토큰입니다.'})


@socketio.on('connect')
@login_required
def handle_connect():
    username = session.get('username')
    sid = request.sid
    sid_to_user[sid] = username
    
    if session.get('is_admin'):
        join_room('admins')
        print(f"Admin connected: {username} ({sid}), joined 'admins' room.")
    else:
        print(f"Employee connected: {username} ({sid}).")

@socketio.on('disconnect')
def handle_disconnect():
    global active_yolo_target_username
    sid = request.sid
    if sid in sid_to_user:
        username = sid_to_user.pop(sid)
        print(f"Client disconnected: {username} ({sid}).")
        if username == active_yolo_target_username:
            active_yolo_target_username = None
            print(f"Active target '{username}' disconnected. Target reset.")

@socketio.on('set_active_target')
@admin_required
def set_active_target(data):
    global active_yolo_target_username
    username = data.get('username')
    active_yolo_target_username = username
    print(f"Admin set new target: {username}")
    emit('target_changed', {'username': username}, room='admins')

@socketio.on('image')
@login_required
def handle_image(data_image):
    current_user = sid_to_user.get(request.sid)

    if session.get('is_admin'):
        return


    if current_user == active_yolo_target_username:

        emit('raw_target_feed', data_image, room='admins')


    if not yolo_lock.acquire(blocking=False):
        emit('response_image', '', to=request.sid)
        return

    try:
        img_bytes = base64.b64decode(data_image.split(',')[1])
        headers = {'Content-Type': 'application/octet-stream'}
        res = requests.post(RPI_DETECT_URL, data=img_bytes, headers=headers, timeout=15)
        
        result_image = '' 
        if res.status_code == 200:
            b64_string = base64.b64encode(res.content).decode('utf-8')
            result_image = 'data:image/jpeg;base64,' + b64_string
        

        emit('response_image', result_image, to=request.sid)

    except requests.exceptions.RequestException as e:
        print(f"객체 탐지 서버 연결 오류: {e}")
        emit('response_image', '', to=request.sid)
    finally:
        yolo_lock.release()

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, 

                 ssl_context=('cert.pem', 'key.pem'), 
                 allow_unsafe_werkzeug=True)
