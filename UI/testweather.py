import pyrebase

firebase_config = {
    "apiKey": "AIzaSy...",
    "authDomain": "environment-app-4a88f.firebaseapp.com",
    "databaseURL": "https://environment-app-4a88f-default-rtdb.firebaseio.com",
    "storageBucket": "environment-app-4a88f.firebasestorage.app",
}

firebase = pyrebase.initialize_app(firebase_config)
db = firebase.database()

try:
    data = db.child("environment").get().val()
    print("Dữ liệu Firebase:", data)
except Exception as e:
    print("Lỗi khi lấy dữ liệu từ Firebase:", e)