USE yolo_webapp_db;

-- =================================================================
-- 1. 직원 및 사용자 계정 생성 (✨ 비밀번호 '1234'로 통일 및 사용자 추가)
-- =================================================================
-- 관리자 (1명)
INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('EMP001', '이순신', '경영지원팀', 1);
INSERT INTO users (employee_id, username, password, email, phone_number)
VALUES (1, 'admin', '1234', 'lee@themoon.com', '010-1111-1111');

-- 기존 일반 직원 (4명)
INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('EMP002', '김유신', 'IT개발팀', 0),
       ('EMP003', '세종대왕', 'IT개발팀', 0),
       ('EMP004', '신사임당', '디자인팀', 0),
       ('EMP005', '강감찬', '영업팀', 0);

INSERT INTO users (employee_id, username, password, email, phone_number)
VALUES (2, 'kim', '1234', 'kim@themoon.com', '010-2222-2222'),
       (3, 'sejong', '1234', 'sejong@themoon.com', '010-3333-3333'),
       (4, 'shin', '1234', 'shin@themoon.com', '010-4444-4444'),
       (5, 'kang', '1234', 'kang@themoon.com', '010-5555-5555');

-- ✨ 신규 일반 직원 (5명)
INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('EMP006', '황진이', '디자인팀', 0),
       ('EMP007', '장영실', 'IT개발팀', 0),
       ('EMP008', '정도전', '경영지원팀', 0),
       ('EMP009', '정몽주', '영업팀', 0),
       ('EMP010', '원효대사', '신사업팀', 0);

INSERT INTO users (employee_id, username, password, email, phone_number)
VALUES (6, 'hwang', '1234', 'hwang@themoon.com', '010-6666-6666'),
       (7, 'jang', '1234', 'jang@themoon.com', '010-7777-7777'),
       (8, 'jeongd', '1234', 'jeongd@themoon.com', '010-8888-8888'),
       (9, 'jeongm', '1234', 'jeongm@themoon.com', '010-9999-9999'),
       (10, 'wonhyo', '1234', 'wonhyo@themoon.com', '010-0000-0000');

-- 모든 직원을 등록 상태로 변경
UPDATE employees SET is_registered = 1 WHERE employee_id BETWEEN 1 AND 10;


-- =================================================================
-- 2. 채팅방 및 대화 내용 생성 (✨ 신규 사용자 참여)
-- =================================================================
-- 그룹 채팅방 (2개)
INSERT INTO chat_rooms (room_type, room_name) VALUES ('group', 'IT개발팀 회의실'); -- room_id: 1
INSERT INTO chat_rooms (room_type, room_name) VALUES ('group', '전사 공지 채널');   -- room_id: 2
-- 1:1 채팅방 (5개)
INSERT INTO chat_rooms (room_type) VALUES ('direct'); -- room_id: 3 (이순신-김유신)
INSERT INTO chat_rooms (room_type) VALUES ('direct'); -- room_id: 4 (세종대왕-신사임당)
INSERT INTO chat_rooms (room_type) VALUES ('direct'); -- room_id: 5 (이순신-강감찬)
INSERT INTO chat_rooms (room_type) VALUES ('direct'); -- room_id: 6 (김유신-장영실)
INSERT INTO chat_rooms (room_type) VALUES ('direct'); -- room_id: 7 (신사임당-황진이)

-- 채팅방 참여자 설정
-- IT개발팀 회의실 (이순신, 김유신, 세종대왕, 장영실)
INSERT INTO participants (room_id, user_id) VALUES (1, 1), (1, 2), (1, 3), (1, 7);
-- 전사 공지 채널 (모두)
INSERT INTO participants (room_id, user_id) VALUES (2, 1), (2, 2), (2, 3), (2, 4), (2, 5), (2, 6), (2, 7), (2, 8), (2, 9), (2, 10);
-- 1:1 채팅방
INSERT INTO participants (room_id, user_id) VALUES (3, 1), (3, 2);
INSERT INTO participants (room_id, user_id) VALUES (4, 3), (4, 4);
INSERT INTO participants (room_id, user_id) VALUES (5, 1), (5, 5);
INSERT INTO participants (room_id, user_id) VALUES (6, 2), (6, 7); -- 김유신-장영실
INSERT INTO participants (room_id, user_id) VALUES (7, 4), (7, 6); -- 신사임당-황진이

-- 메시지 내용
INSERT INTO messages (room_id, sender_id, content) VALUES
(1, 2, '안녕하세요, 오늘 회의 자료 공유드립니다.'),
(1, 7, '네, 확인하겠습니다.'),
(1, 3, '네, 확인했습니다. 감사합니다.'),
(1, 1, '모두 수고 많으십니다.'),
(3, 1, '김유신 팀장, 주간 보고서 제출 부탁드립니다.'),
(3, 2, '네, 알겠습니다. 오늘 중으로 제출하겠습니다.'),
(4, 4, '세종대왕님, 시안 검토 부탁드려요~'),
(4, 3, '훌륭하군요. 바로 컨펌하겠습니다.'),
(6, 2, '장영실님, 신규 서버 스펙 검토 부탁합니다.'),
(6, 7, '네 팀장님, 검토 후 의견 드리겠습니다.');


-- =================================================================
-- 3. 커뮤니티 (공지사항, 게시판, 댓글) 생성 (✨ 신규 사용자 활동 추가)
-- =================================================================
-- 공지사항 (관리자 작성)
INSERT INTO notices (user_id, title, content) VALUES
(1, '정기 서버 점검 일정 안내 (10/15)', '안녕하세요, TheMoon입니다. 보다 안정적인 서비스 제공을 위해 아래와 같이 서버 점검을 진행할 예정입니다. [내용] ...'),
(1, '2025년도 연말정산 일정 공지', '2025년 귀속 연말정산 관련 일정을 안내해 드립니다. 직원 여러분께서는 기간 내에 서류를 제출해주시기 바랍니다.');

-- 익명 게시판 글
INSERT INTO posts (user_id, title, content) VALUES
(3, '1층 커피머신 고장났어요', '아침부터 커피를 못 마시니 힘드네요. 빠른 수리 부탁드립니다!'),
(5, '업무 효율화 아이디어 제안', '반복적인 보고 업무를 자동화하는 스크립트를 만들어보면 어떨까요?'),
(4, '사내 점심 메뉴 추천 받습니다', '요즘 점심 메뉴 고르기가 너무 힘드네요. 맛집 아시는 분 공유좀 해주세요~'),
(8, '출퇴근 기록 오류 문의', '오늘 아침 출근 기록이 누락된 것 같습니다. 확인 부탁드립니다.'); -- 신규 사용자

-- 댓글
INSERT INTO comments (post_id, user_id, content) VALUES
(1, 2, '저도 아침에 당황했네요. 빨리 고쳐졌으면...'),
(1, 1, '경영지원팀에 전달하여 조치하도록 하겠습니다.'),
(2, 7, '좋은 아이디어 같아요! 저도 동참하고 싶습니다.'), -- 신규 사용자
(3, 3, '회사 근처 새로 생긴 파스타집 맛있어요!'),
(4, 1, '정도전님, 인사팀에 확인해보겠습니다.');


-- =================================================================
-- 4. 업무 요청 시스템 데이터 생성 (✨ 신규 사용자에게 업무 할당)
-- =================================================================
-- 업무 요청 (관리자 -> 직원)
INSERT INTO work_requests (user_id, title, content, status, due_date) VALUES
(1, '4분기 실적 보고서 제출', '영업팀 4분기 실적 데이터를 취합하여 보고서 형식으로 제출 바랍니다.', '처리중', '2025-10-25'),
(1, '신규 프로젝트 로고 디자인 시안', '하반기 신규 프로젝트에 사용할 로고 디자인 시안 3종을 요청합니다.', '접수', '2025-10-31'),
(1, '서버 보안 패치 작업', '운영 서버의 보안 취약점 패치 작업을 진행해주세요.', '완료', '2025-10-09'),
(1, '신사업 아이템 시장 조사', '신사업팀은 미래 성장 동력 발굴을 위한 시장 조사를 진행하고 보고 바랍니다.', '접수', '2025-11-15');

-- 업무 담당자 할당
INSERT INTO work_assignees (request_id, assignee_id) VALUES
(1, 5), (1, 9), -- 실적 보고서 -> 강감찬, 정몽주
(2, 4), (2, 6), -- 로고 디자인 -> 신사임당, 황진이
(3, 2), (3, 7), -- 서버 보안 -> 김유신, 장영실
(4, 10);      -- 시장 조사 -> 원효대사

-- 첨부 파일
INSERT INTO attachments (related_type, related_id, original_filename, stored_filename, file_path, file_size) VALUES
('work_request', 1, '보고서_양식.docx', 'report_form_rev1.docx', 'C:/server_uploads/', 15360),
('work_request', 1, '참고_데이터.xlsx', 'ref_data_2025_q3.xlsx', 'C:/server_uploads/', 48220),
('work_request', 2, '프로젝트_개요.pdf', 'project_overview.pdf', 'C:/server_uploads/', 120480);

COMMIT;

