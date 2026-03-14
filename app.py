from flask import Flask, request, send_file, jsonify
import subprocess
import json
import os
import webbrowser
import threading
import time

app = Flask(__name__, static_folder='.', static_url_path='')

@app.route("/")
def home():
    return send_file("index.html", mimetype='text/html')

@app.route("/generate", methods=['GET', 'POST'])
def generate():
    try:
        size = request.args.get("size", "1")
        text = request.args.get("text", "Hello World")

        if not text or not text.strip():
            return jsonify({"error": "Text input is empty"}), 400
        
        if size not in ["1", "2", "3"]:
            return jsonify({"error": "Invalid key size"}), 400

        exe_path = os.path.abspath("encrypt.exe")

        if not os.path.exists(exe_path):
            return jsonify({"error": f"encrypt.exe not found"}), 500

        result = subprocess.run(
            [exe_path, size, text],
            capture_output=True,
            timeout=10,
            text=True
        )

        if result.returncode != 0:
            error_msg = result.stderr.strip()
            return jsonify({"error": f"C++ process failed: {error_msg}"}), 500

        output = result.stdout.strip()
        
        if not output:
            return jsonify({"error": "C++ returned empty output"}), 500

        try:
            json_data = json.loads(output)
            return jsonify(json_data), 200
        except json.JSONDecodeError as e:
            return jsonify({"error": f"Invalid JSON: {str(e)}"}), 500

    except subprocess.TimeoutExpired:
        return jsonify({"error": "C++ process timeout"}), 500
    except Exception as e:
        return jsonify({"error": f"Error: {str(e)}"}), 500

# افتح المتصفح تلقائياً عند البدء
def open_browser():
    time.sleep(1.5)
    webbrowser.open('http://127.0.0.1:5000')

if __name__ == "__main__":
    # افتح المتصفح في thread منفصلة
    threading.Thread(target=open_browser, daemon=True).start()
    
    # شغّل Flask
    print("🚀 البرنامج يبدأ...")
    print("🌐 سيفتح المتصفح تلقائياً على http://127.0.0.1:5000")
    app.run(debug=False, host='127.0.0.1', port=5000)