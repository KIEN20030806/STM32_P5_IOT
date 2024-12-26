import requests
import pyrebase
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.image import Image
from kivy.uix.button import Button
from kivy.graphics import Color, RoundedRectangle, Rectangle
from kivy.clock import Clock
from datetime import datetime
from kivy.uix.button import Button


# Firebase Configuration
firebase_config = {
    "apiKey": "AIzaSyC82ROzBE8yIuIY1TX6-XCnr6R9TBNdRTE",
    "authDomain": "environment-app-4a88f.firebaseapp.com",
    "databaseURL": "https://environment-app-4a88f-default-rtdb.firebaseio.com",
    "storageBucket": "environment-app-4a88f.firebasestorage.app",
}

# Initialize Firebase
firebase = pyrebase.initialize_app(firebase_config)
db = firebase.database()

# API Key and Base URL
WEATHER_API_URL = "https://api.weatherapi.com/v1/current.json"
API_KEY = "d35945be118e4fd7b3971209241912"
CITY = "ho chi minh city"

def fetch_weather_data():
    """Lấy dữ liệu thời tiết từ API"""
    try:
        response = requests.get(
            WEATHER_API_URL, params={"key": API_KEY, "q": CITY}
        )
        if response.status_code == 200:
            data = response.json()
            return {
                "wind_kph": data["current"]["wind_kph"],
                "cloud": data["current"]["cloud"],
                "vis_km": data["current"]["vis_km"],
                "uv": data["current"]["uv"],
                "precip_mm": data["current"]["precip_mm"],
            }
        else:
            print(f"Lỗi khi lấy dữ liệu thời tiết: {response.status_code}")
            return None
    except Exception as e:
        print(f"Exception while fetching weather data: {e}")
        return None

def update_firebase(weather_data):
    """Cập nhật Firebase với dữ liệu thời tiết"""
    try:
        db.child("environment").update({
            "wind": weather_data["wind_kph"],
            "cloud": weather_data["cloud"],
            "vis": weather_data["vis_km"],
            "uv": weather_data["uv"],
            "rain": weather_data["precip_mm"]
        })
    except Exception as e:
        print(f"Lỗi khi cập nhật Firebase: {e}")

class InfoCard(BoxLayout):
    def __init__(self, icon_path, title, value, value_color, **kwargs):
        super().__init__(**kwargs)
        self.orientation = 'vertical'
        self.padding = 10
        self.spacing = 10
        self.size_hint = (1, None)
        self.height = 100

        # Background styling (màu #99FFFF cho các ô)
        with self.canvas.before:
            Color(1, 1, 1, 1)  # Màu #99FFFF
            self.bg = RoundedRectangle(size=self.size, pos=self.pos, radius=[15])
            self.bind(size=self.update_bg, pos=self.update_bg)

        # Icon
        self.add_widget(Image(
            source=icon_path,
            size_hint=(1, 0.5),
            allow_stretch=True
        ))

        # Title
        self.add_widget(Label(
            text=title,
            font_size='14sp',
            color=(0.2, 0.2, 0.2, 1),
            size_hint=(1, 0.25)
        ))

        # Value
        self.value_label = Label(
            text=value,
            font_size='16sp',
            bold=True,
            color=value_color,
            size_hint=(1, 0.25)
        )
        self.add_widget(self.value_label)

    def update_value(self, value):
        self.value_label.text = value

    def update_bg(self, *args):
        self.bg.size = self.size
        self.bg.pos = self.pos

#Định dạng button
class SquareButton(Button):
    def __init__(self, background_color=(1, 0.5, 0.5, 1), **kwargs):
        super().__init__(**kwargs)
        self.size_hint = (None, None)
        self.size = (40, 40)  # Kích thước của nút vuông

        # Gán các sự kiện
        self.bind(on_press=self.on_button_press)
        self.bind(on_release=self.on_button_release)

        # Thiết lập màu nền
        with self.canvas.before:
            Color(1, 0.5, 0.5, 1)  # Sử dụng màu được chỉ định
            self.rect = Rectangle(pos=self.pos, size=self.size)
        
        self.bind(pos=self.update_rect, size=self.update_rect)

    def update_rect(self, *args):
        self.rect.pos = self.pos
        self.rect.size = self.size

    def on_button_press(self, instance):
        print("Nút đã được nhấn!")
        # Thay đổi màu sắc hoặc thực hiện hành động khác
        self.canvas.before.clear()
        with self.canvas.before:
            Color(1, 0, 0, 1)  # Màu đỏ khi nhấn
            self.rect = Rectangle(pos=self.pos, size=self.size)

    def on_button_release(self, instance):
        print("Nút đã được thả ra!")
        # Khôi phục màu sắc ban đầu
        self.canvas.before.clear()
        with self.canvas.before:
            Color(1, 0.5, 0.5, 1)  # Màu đỏ nhạt khi không nhấn
            self.rect = Rectangle(pos=self.pos, size=self.size)

class ButtonCard(BoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = 'vertical'
        self.padding = 10
        self.spacing = 10
        self.size_hint = (1, None)
        self.height = 90  # Tăng chiều cao để chứa các nút vuông

        # Biến để lưu giá trị cho các nút
        self.mod = "0"

        with self.canvas.before:
            Color(1, 1, 1, 1)  # Màu nền trắng
            self.bg = RoundedRectangle(size=self.size, pos=self.pos, radius=[15])
            self.bind(size=self.update_bg, pos=self.update_bg)

        # Tiêu đề cho card
        self.add_widget(Label(
            text="Chọn chế độ hiển thị trên bảng Led",
            font_size='16sp',
            bold=True,
            color=(0.2, 0.2, 0.2, 1),
            size_hint=(1, 0.2)
        ))

        # Tạo BoxLayout để chứa các nút theo chiều ngang
        button_layout = BoxLayout(orientation='horizontal', spacing=10, size_hint=(1, 0.5), padding=[20, 0, 20, 0])

        # Tạo 4 nút vuông với màu sắc khác nhau
        # Tạo các nút vuông và gán sự kiện
        self.button1 = SquareButton(text="1", background_color=(1, 0.5, 0.5, 1), size_hint=(None, None))
        self.button1.bind(on_press=self.on_button1_press)
        
        self.button2 = SquareButton(text="2", background_color=(0, 1, 0, 1), size_hint=(None, None))
        self.button2.bind(on_press=self.on_button2_press)
        
        self.button3 = SquareButton(text="3", background_color=(0, 0, 1, 1), size_hint=(None, None))
        self.button3.bind(on_press=self.on_button3_press)
        
        self.button4 = SquareButton(text="4", background_color=(1, 1, 0, 1), size_hint=(None, None))
        self.button4.bind(on_press=self.on_button4_press)

        # Thay đổi kích thước của các nút
        self.button1.size = (40, 40)
        self.button2.size = (40, 40)
        self.button3.size = (40, 40)
        self.button4.size = (40, 40)

        # Thêm các nút vào layout
        button_layout.add_widget(Label(size_hint=(0.1, 1)))  # Thêm một Label trống để tạo khoảng trống bên trái
        button_layout.add_widget(self.button1)
        button_layout.add_widget(self.button2)
        button_layout.add_widget(self.button3)
        button_layout.add_widget(self.button4)
        button_layout.add_widget(Label(size_hint=(0.1, 1)))  # Thêm một Label trống để tạo khoảng trống bên trái

        # Thêm button_layout vào card
        self.add_widget(button_layout)

    def update_bg(self, *args):
        self.bg.size = self.size
        self.bg.pos = self.pos
    

    def on_button1_press(self, instance):
        self.mod = 1
        self.update_firebase_mods()  # Cập nhật Firebase
        print(f"mod: {self.mod}")

    def on_button2_press(self, instance):
        self.mod = 2
        self.update_firebase_mods()  # Cập nhật Firebase
        print(f"mod: {self.mod}")

    def on_button3_press(self, instance):
        self.mod = 3
        self.update_firebase_mods()  # Cập nhật Firebase
        print(f"mod: {self.mod}")

    def on_button4_press(self, instance):
        self.mod = 4
        self.update_firebase_mods()  # Cập nhật Firebase
        print(f"mod: {self.mod}")
    
    def update_firebase_mods(self):
        try:
            db.child("environment").update({
                "mod": self.mod  # Chỉ cập nhật một mod
            })
            print("Cập nhật Firebase thành công:", self.mod)
        except Exception as e:
            print(f"Lỗi khi cập nhật Firebase: {e}")

#thời tiết
class WeatherCard(BoxLayout):
    def __init__(self, weather_info, **kwargs):
        super().__init__(**kwargs)
        self.orientation = 'vertical'
        self.padding = 10
        self.spacing = 10
        self.size_hint = (1, None)
        self.height = 100

        # Background styling (màu #99FFFF cho ô lớn)
        with self.canvas.before:
            Color(1, 1, 1, 1)  # Màu #99FFFF
            self.bg = RoundedRectangle(size=self.size, pos=self.pos, radius=[15])
            self.bind(size=self.update_bg, pos=self.update_bg)

        # Weather icon and description
        self.add_widget(Label(
            text="Thời tiết hiện tại",
            font_size='16sp',
            bold=True,
            color=(0.2, 0.2, 0.2, 1),
            size_hint=(1, 0.2)
        ))
        self.weather_info_label = Label(
            text=weather_info,
            font_size='14sp',
            color=(0, 0, 1, 1),
            size_hint=(1, 0.8)
        )
        self.add_widget(self.weather_info_label)

    def update_weather(self, weather_info):
        """Cập nhật thông tin thời tiết"""
        self.weather_info_label.text = weather_info
        
    def update_bg(self, *args):
        self.bg.size = self.size
        self.bg.pos = self.pos


class InfoScreen(BoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = 'vertical'
        self.padding = 20
        self.spacing = 10

        # Background color for the entire screen (trắng)
        with self.canvas.before:
            Color(0.972, 0.972, 1, 1)  # White
            self.bg = Rectangle(size=self.size, pos=self.pos)
            self.bind(size=self.update_bg, pos=self.update_bg)

        # Header
        self.add_widget(Label(
            text="Thông tin môi trường",
            font_size='20sp',
            bold=True,
            size_hint=(1, 0.1),
            color=(0, 0, 0, 1)  # Màu đen
        ))

        # Main content: 6 small cards + 1 large card for weather
        self.grid_layout = GridLayout(cols=2, spacing=20, size_hint=(1, 0.6))

        # Adding small information cards
        self.temp_card = InfoCard("temperature_icon.png", "Nhiệt độ", "Đang tải...", (1, 0, 0, 1))
        self.hum_card = InfoCard("humidity_icon.png", "Độ ẩm", "Đang tải...", (0, 0, 1, 1))
        self.air_card = InfoCard("atmosphere_icon.png", "Độ ô nhiễm", "Đang tải...", (0, 0.5, 0.5, 1))
        self.noise_card = InfoCard("noise_icon.png", "Tiếng ồn", "Đang tải...", (1, 0.5, 0, 1))
        self.light_card = InfoCard("light_icon.png", "Ánh sáng", "Đang tải...", (1, 1, 0, 1))
        self.clock_card = InfoCard("clock_icon.png","Thời gian hiện tại", "Đang tải...", (1, 0.5, 1, 1))

        self.grid_layout.add_widget(self.temp_card)
        self.grid_layout.add_widget(self.hum_card)
        self.grid_layout.add_widget(self.air_card)
        self.grid_layout.add_widget(self.noise_card)
        self.grid_layout.add_widget(self.light_card)
        self.grid_layout.add_widget(self.clock_card)

        self.add_widget(self.grid_layout)

        # Khởi tạo ButtonCard
        self.button_card = ButtonCard()
        # Thêm ButtonCard vào layout
        self.add_widget(self.button_card)
        # Adding large weather card
        self.weather_card = WeatherCard("Đang tải dữ liệu thời tiết...")
        self.add_widget(self.weather_card)

        # Notification card at the bottom
        # Thêm Label cho thông báo
        self.notification_label = Label(
            text="",
            font_size='16sp',
            color=(1, 0, 0, 1),  # Màu đỏ cho thông báo
            size_hint=(1, 0.15)  
        )
        
        # Tạo khung nền cho thông báo
        with self.canvas.before:
            Color(1, 1, 1, 1)  # Màu nền trắng
            self.notification_bg = RoundedRectangle(size=self.notification_label.size, pos=self.notification_label.pos, radius=[15])
            self.notification_label.bind(size=self.update_notification_bg, pos=self.update_notification_bg)
        
        self.notification_label.bind(size=self.update_notification_bg, pos=self.update_notification_bg)
        self.add_widget(self.notification_label)

        # Thêm phương thức để cập nhật nền cho thông báo
    def update_notification_bg(self, *args):
        self.notification_bg.size = self.notification_label.size
        self.notification_bg.pos = self.notification_label.pos
        
        # Schedule to update data from Firebase every 1 seconds
        Clock.schedule_interval(self.update_all_data, 1)
    def show_notification(self, dt):
        # Lấy dữ liệu từ Firebase
        data = db.child("environment").get().val()

        # Định nghĩa ngưỡng tiêu chuẩn
        self.uv_threshold = 6
        self.rain_threshold = 50
        self.pollution_threshold = 40
        self.temperature_threshold = 40
        self.noise_threshold = 50
        # Lấy các giá trị từ dữ liệu
        uv = data.get('uv', 0)
        rain = data.get('rain', 0)
        atmos = data.get('atmos', 0)
        tem = data.get('tem', 0)
        noise = data.get('noise',0)
        # Xây dựng thông điệp cảnh báo
        messages = []
        if uv > self.uv_threshold:
            messages.append(f"UV: {uv} (vượt ngưỡng {self.uv_threshold})")
        if rain > self.rain_threshold:
            messages.append(f"Lượng mưa: {rain} mm (vượt ngưỡng {self.rain_threshold} mm)")
        if atmos > self.pollution_threshold:
            messages.append(f"Độ ô nhiễm: {atmos}% (vượt ngưỡng {self.pollution_threshold}%)")
        if tem > self.temperature_threshold:
            messages.append(f"Nhiệt độ: {tem}°C (vượt ngưỡng {self.temperature_threshold}°C)")
        if noise > self.noise_threshold:
            messages.append(f"Tiếng ồn: {tem}dB (vượt ngưỡng {self.temperature_threshold}dB)")

        # Kiểm tra xem có thông số nào vượt ngưỡng không
        if messages:
            message = "Cảnh báo: Các thông số vượt quá ngưỡng an toàn!\n" + "\n".join(messages)
            self.notification_label.color = (1, 0, 0, 1)  # Đặt màu đỏ cho cảnh báo
        else:
            message = "Thời tiết đang tốt."
            self.notification_label.color = (0, 1, 0, 1)  # Đặt màu xanh cho thông báo tốt
        # Cập nhật thông báo vào Label
        self.notification_label.text = message
    #Đồng bộ dữ liệu của weather và data
    def update_all_data(self, dt):
        self.update_weather_info(dt)
        self.update_data(dt)
        self.update_clock(dt)  # Cập nhật thời gian
        self.update_bg(dt) # Cập nhật nút
        self.show_notification(dt)  # Hiển thị thông báo
        self.update_notification_bg(dt)
    def update_clock(self, dt):
        current_time = datetime.now().strftime("%d-%m-%Y  %H:%M:%S")
        self.clock_card.update_value(current_time)

    #Tải dữ liệu thời tiết        
    def update_weather_info(self, dt):
        weather_data = fetch_weather_data()
        if weather_data:
            weather_info = (f"Gió: {weather_data['wind_kph']} km/h, "
                            f"Mây: {weather_data['cloud']}%, "
                            f"Tầm nhìn: {weather_data['vis_km']} km, "
                            f"UV: {weather_data['uv']}"
                            f"Lượng mưa: {weather_data['precip_mm']} mm")
            self.weather_card.update_weather(weather_info)
            update_firebase(weather_data) 

    #Tải dữ liệu thời gian       
    def update_clock(self, dt):
        current_time = datetime.now().strftime("%d-%m-%Y  %H:%M:%S")
        self.clock_card.update_value(current_time)
        
        # Cập nhật current_time vào Firebase
        try:
            db.child("environment").update({"time": current_time})
        except Exception as e:
            print(f"Lỗi khi cập nhật thời gian vào Firebase: {e}")
    def update_data(self, dt):
        try:
            # Lấy dữ liệu từ Firebase
            data = db.child("environment").get().val()

            # Update information cards with new data
            self.temp_card.update_value(f"{data['tem']}°C" if 'tem' in data else "N/A")
            self.hum_card.update_value(f"{data['hum']}%" if 'hum' in data else "N/A")
            self.air_card.update_value(f"{data['atmos']}%" if 'atmos' in data else "N/A")
            self.noise_card.update_value(f"{data['noise']} dB" if 'noise' in data else "N/A")
            self.light_card.update_value(f"{data['light']} Lux" if 'light' in data else "N/A")
            self.clock_card.update_value(f"{data['time']} " if 'time' in data else "N/A")
       
        except Exception as e:
            print(f"Lỗi khi lấy dữ liệu từ Firebase: {e}")

    def update_bg(self, *args):
        self.bg.size = self.size
        self.bg.pos = self.pos


class EnvironmentApp(App):
    def build(self):
        return InfoScreen()


if __name__ == "__main__":
    EnvironmentApp().run()