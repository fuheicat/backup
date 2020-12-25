from flask import Flask, request
import os
import json


app = Flask(__name__)


@app.route('/filelist', methods=['GET'])
def filelist():
    return json.dumps([x for x in os.listdir('.') if os.path.isfile(x) and x != "server.py"])


@app.route('/file/<filename>', methods=['PUT'])
def add(filename):
    with open(filename, 'w') as f:
        f.write(str(request.get_data(), encoding="utf-8"))
        return 'success'
    return 'error'


@app.route('/file/<filename>', methods=['DELETE'])
def delete(filename):
    if os.path.exists(filename):
        os.remove(filename)
        return 'success'
    return 'error'


@app.route('/file/<filename>', methods=['GET'])
def get(filename):
    if os.path.exists(filename):
        with open(filename, 'r') as f:
            return f.read()
    return ''


if __name__ == "__main__":
    app.run(host='0.0.0.0')
