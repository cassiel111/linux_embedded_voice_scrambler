import os
import subprocess
import pexpect
from flask import Flask, render_template, jsonify, send_from_directory

app = Flask(__name__)

# ====================== 在这里改你的信息 ======================
BOARD_IP = "192.168.137.10"
BOARD_USER = "root"
BOARD_PWD = "123456"
BOARD_AUDIO_DIR = "/root/"
LOCAL_SAVE_DIR = os.path.expanduser("~/qtworks/soundsback")
# ==============================================================

os.makedirs(LOCAL_SAVE_DIR, exist_ok=True)

@app.route('/')
def index():
    return render_template('index.html')

# 检测开发板是否连通
@app.route('/api/check')
def check():
    try:
        res = subprocess.run(
            ['ping', '-c', '1', '-W', '1', BOARD_IP],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        return jsonify(online=res.returncode == 0)
    except Exception:
        return jsonify(online=False)

# 自动输密码 scp 拉取音频
@app.route('/api/pull')
def pull():
    remote_files = f"{BOARD_USER}@{BOARD_IP}:{BOARD_AUDIO_DIR}*.wav"
    cmd = f"scp {remote_files} {LOCAL_SAVE_DIR}"

    try:
        child = pexpect.spawn(cmd, timeout=15)
        # 匹配是否要确认主机密钥 / 输入密码
        i = child.expect([r"\(yes/no\)\?", r"[Pp]assword:"], timeout=5)

        if i == 0:
            child.sendline("yes")
            child.expect(r"[Pp]assword:")

        child.sendline(BOARD_PWD)
        child.wait()
    except Exception as e:
        print("scp 拉取失败:", e)

    wavs = [f for f in os.listdir(LOCAL_SAVE_DIR) if f.endswith('.wav')]
    return jsonify(files=wavs)

# 网页访问音频文件
@app.route('/audio/<filename>')
def audio(filename):
    return send_from_directory(LOCAL_SAVE_DIR, filename)

# 获取本地所有音频文件列表
@app.route('/api/audio_list')
def audio_list():
    wavs = [f for f in os.listdir(LOCAL_SAVE_DIR) if f.endswith('.wav')]
    return jsonify(list=wavs)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8090, debug=True)
