$(document).ready(function() {
    const video = document.getElementById('video');
    const canvas = document.getElementById('canvas');
    const context = canvas.getContext('2d');
    const processedImage = document.getElementById('processed_image');
    
    // 현재 서버가 이미지를 처리 중인지 상태를 저장하는 변수
    let isProcessing = false;

    // 1. Socket.IO 서버 연결
    const socket = io();
    socket.on('connect', () => {
        console.log('Socket.IO 서버에 연결되었습니다.');
    });

    // 2. 웹캠 설정
    async function setupCamera() {
        try {
            const stream = await navigator.mediaDevices.getUserMedia({
                video: { width: 640, height: 480 },
                audio: false
            });
            video.srcObject = stream;
        } catch (err) {
            console.error("웹캠을 사용할 수 없습니다:", err);
            alert("웹캠을 사용할 수 없습니다. 권한을 확인해주세요.");
        }
    }
    setupCamera();

    // 3. 주기적으로 서버로 이미지 전송
    setInterval(() => {
        // 서버가 다른 작업을 하고 있지 않을 때만 이미지를 보냄
        if (!isProcessing && video.readyState === video.HAVE_ENOUGH_DATA) {
            isProcessing = true; // "처리 중" 상태로 변경
            
            canvas.width = video.videoWidth;
            canvas.height = video.videoHeight;
            context.drawImage(video, 0, 0, canvas.width, canvas.height);
            
            const data = canvas.toDataURL('image/jpeg', 0.7);
            socket.emit('image', data);
        }
    }, 200); // 0.2초 간격

    // 4. 서버로부터 처리된 이미지를 수신
    socket.on('response_image', function(data) {
        // 서버로부터 받은 데이터가 비어있지 않을 때만 이미지를 업데이트합니다.
        if (data) {
            processedImage.src = data;
        }

        // "처리 완료" 상태로 변경하여 다음 이미지 전송을 준비합니다.
        isProcessing = false;
    });

    // --- 상태 변경 버튼 로직 (기존과 동일) ---
    function updateStatus(newStatus) {
        $.ajax({
            url: '/api/update_status',
            type: 'POST',
            contentType: 'application/json',
            data: JSON.stringify({ status: newStatus }),
            success: function(response) {
                console.log(response.message);
            },
            error: function() {
                alert('상태 변경에 실패했습니다.');
            }
        });
    }
    $('#btn-online').on('click', function() { updateStatus('online'); });
    $('#btn-away').on('click', function() { updateStatus('away'); });
});

