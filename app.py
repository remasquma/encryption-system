from flask import Flask, request, send_file, jsonify
import subprocess
import json
import os
import sys

app = Flask(__name__)

@app.route("/")
def home():
    return send_file("index.html")

@app.route("/generate")
def generate():
    size = request.args.get("size", "1")
    text = request.args.get("text", "Hello World")

    if not text or not text.strip():
        return jsonify({"error": "Text input is empty"}), 400
    
    if size not in ["1", "2", "3"]:
        return jsonify({"error": "Invalid key size"}), 400

    try:
        # تشغيل encrypt.exe
        exe_path = os.path.join(os.getcwd(), "encrypt.exe")
        
        result = subprocess.run(
            [exe_path, size, text],
            capture_output=True,
            timeout=10,
            text=True
        )

        if result.returncode != 0:
            return jsonify({"error": result.stderr}), 500

        # محاولة تحويل لـ JSON
        try:
            return jsonify(json.loads(result.stdout))
        except:
            return jsonify({"result": result.stdout})

    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    # مهم: نستخدم PORT من Render
    port = int(os.environ.get("PORT", 5000))
    app.run(host="0.0.0.0", port=port, debug=False)
