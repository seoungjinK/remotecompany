document.addEventListener('DOMContentLoaded', function() {
    // 실시간 직원 상태 업데이트 함수
    function updateUserStatuses() {
        fetch('/api/get_statuses')
            .then(response => response.json())
            .then(statuses => {
                for (const username in statuses) {
                    const status = statuses[username] || 'offline';
                    const userLi = document.querySelector(`.user-item[data-username="${username}"]`);
                    if (userLi) {
                        const dot = userLi.querySelector('.status-dot');
                        dot.className = 'status-dot status-' + status;
                    }
                }
            })
            .catch(error => console.error('상태 업데이트 중 오류 발생:', error));
    }
    setInterval(updateUserStatuses, 5000); // 5초마다 상태 업데이트
    updateUserStatuses(); // 페이지 로드 시 즉시 실행

    // 메신저 실행 버튼 이벤트
    document.getElementById('run-mfc-btn').addEventListener('click', function() {
        alert('메신저 프로그램을 서버에서 실행합니다.');
        fetch('/run-mfc', { method: 'POST' })
            .then(res => res.json())
            .then(data => {
                if(data.status !== 'success') alert('오류: ' + data.message);
            })
            .catch(err => alert('클라이언트 측 오류가 발생했습니다.'));
    });

    // --- 실시간 영상 관련 로직 ---
    const socket = io();
    const userItems = document.querySelectorAll('.user-item');
    const watchingTitle = document.getElementById('watching-title');
    const processedVideo = document.getElementById('processed_video');

    socket.on('connect', () => {
        console.log('관리자 소켓 서버에 연결되었습니다.');
    });

    // 각 사용자 아이템에 클릭 이벤트 추가
    userItems.forEach(item => {
        item.addEventListener('click', () => {
            const username = item.dataset.username;
            const displayName = item.querySelector('.username').textContent;
            
            // 모든 아이템에서 'active' 클래스 제거 후 클릭된 아이템에 추가
            userItems.forEach(i => i.classList.remove('active'));
            item.classList.add('active');

            // 서버에 타겟 사용자 변경 요청
            socket.emit('set_active_target', { username: username });
            
            // UI 업데이트
            watchingTitle.textContent = `${displayName} 님의 실시간 원본 영상`;
            processedVideo.src = ''; // 이전 영상 초기화
        });
    });

    // 서버로부터 원본 영상을 수신하여 화면에 표시
    socket.on('raw_target_feed', function(data) {
        if (data) {
            processedVideo.src = data;
        }
    });

    // 다른 관리자가 타겟을 변경했을 때 내 화면도 동기화
    socket.on('target_changed', function(data) {
        const username = data.username;
        const targetItem = document.querySelector(`.user-item[data-username="${username}"]`);
        if (targetItem) {
            const displayName = targetItem.querySelector('.username').textContent;
            watchingTitle.textContent = `${displayName} 님의 실시간 원본 영상`;
            userItems.forEach(item => {
                item.classList.toggle('active', item.dataset.username === username);
            });
        }
    });
});