from flask import Flask, jsonify, request

import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import json


cred = credentials.Certificate('yedped-cf694-firebase-adminsdk-xpzw5-c1fc3de6da.json')

firebase_admin.initialize_app(cred, {
	'databaseURL' : 'https://yedped-cf694-default-rtdb.firebaseio.com/'
})

app = Flask(__name__)



@app.route('/data/write', methods=['POST'])
def write_data():
	data = request.get_json()
	ref_db = db.reference('/yedped/')
	ref_db.child('Logs').push(data)
	return 'done'

@app.route('/data/read/all', methods=['GET'])
def read_all_data():
    json_data = request.get_json(silent=True)
    ref = db.reference('/yedped/')
    return jsonify(ref.get())

if __name__ == '__main__':
	app.run(debug=True, host="0.0.0.0", port=1234)