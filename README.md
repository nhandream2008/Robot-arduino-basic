# Xe robot thông minh Arduino

Xe robot 3 bánh điều khiển bằng Bluetooth hoặc remote hồng ngoại, có chế độ tự động tránh vật cản. Dự án thiết kế cho Arduino Uno R3, Shield L293D V1, HC-05, HC-SR04, servo SG90 và mắt thu IR.

## Tính năng

- **Bluetooth:** điều khiển xe bằng ứng dụng điện thoại qua HC-05.
- **Remote hồng ngoại:** điều khiển bằng các phím mũi tên và phím OK.
- **Tự động:** xe dò vật cản, lùi, quét hai hướng bằng servo rồi chọn hướng để rẽ.
- **An toàn:** khi đang tiến ở chế độ điều khiển tay, xe dừng nếu khoảng cách quá gần hoặc cảm biến siêu âm không phản hồi.
- **Xử lý không chặn:** các thao tác chờ trong chế độ tự động dùng `millis()` để vẫn nhận lệnh đổi chế độ và dừng xe.

## Phần cứng

- Arduino Uno R3
- Shield điều khiển động cơ L293D V1
- Khung xe robot 3 bánh, 2 động cơ DC
- Module Bluetooth HC-05
- Cảm biến siêu âm HC-SR04
- Servo SG90
- Bộ thu hồng ngoại và remote
- Nguồn cấp phù hợp cho Arduino, động cơ và servo

## Đấu nối

| Thiết bị     | Chân thiết bị   | Chân Arduino / Shield |
| ------------ | --------------- | --------------------- |
| Động cơ trái | Hai dây động cơ | Cổng M3               |
| Động cơ phải | Hai dây động cơ | Cổng M4               |
| Servo SG90   | Tín hiệu        | D10 / header SERVO1   |
| HC-SR04      | Trig            | A0                    |
| HC-SR04      | Echo            | A1                    |
| HC-SR04      | VCC, GND        | 5V, GND               |
| HC-05        | TX              | A2 (Arduino nhận)     |
| HC-05        | RX              | A3 (Arduino truyền)   |
| HC-05        | VCC, GND        | 5V, GND               |
| Bộ thu IR    | OUT / DAT       | D2                    |
| Bộ thu IR    | VCC, GND        | 5V, GND               |

**Lưu ý:** nên dùng cầu chia áp cho đường tín hiệu Arduino TX (A3) sang RX của HC-05 vì RX của HC-05 dùng mức logic 3,3 V. GND của các module phải nối chung. Kiểm tra đúng chân tín hiệu, nguồn và chiều cắm servo trước khi cấp điện.

## Thư viện cần cài

Cài các thư viện sau trong Arduino IDE Library Manager:

- **Adafruit Motor Shield library** phiên bản V1, cung cấp `AFMotor.h`.
- **Arduino-IRremote** phiên bản 4.x, cung cấp `IRremote.hpp`.

`Servo` và `SoftwareSerial` thường có sẵn trong Arduino IDE.

## Cài đặt và nạp chương trình

1. Đặt file chương trình `xe_thong_minh.ino` cùng tên với thư mục sketch Arduino. Ví dụ: `xe_thong_minh/xe_thong_minh.ino`.
2. Cài các thư viện được liệt kê ở trên.
3. Mở sketch bằng Arduino IDE.
4. Chọn board **Arduino Uno** và đúng cổng COM.
5. Nạp chương trình.
6. Khi thử lần đầu, kê bánh xe lên khỏi mặt đất để kiểm tra chiều quay và lệnh dừng.

## Điều khiển Bluetooth

HC-05 mặc định thường dùng baud rate `9600`. Ứng dụng điện thoại cần gửi ký tự đơn tương ứng:

| Ký tự   | Chức năng                                                        |
| ------- | ---------------------------------------------------------------- |
| `X`     | Chuyển sang chế độ Bluetooth                                     |
| `I`     | Chuyển sang chế độ remote IR                                     |
| `A`     | Chuyển sang chế độ tự động                                       |
| `F`     | Tiến                                                             |
| `B`     | Lùi                                                              |
| `L`     | Rẽ trái tại chỗ                                                  |
| `R`     | Rẽ phải tại chỗ                                                  |
| `S`     | Dừng; khi đang tự động sẽ dừng và chuyển về điều khiển Bluetooth |
| `0`     | Dừng xe / đặt tốc độ tay về 0                                    |
| `1`–`9` | Chọn tốc độ tay từ thấp đến cao                                  |

Chương trình cũng nhận `q` để chọn tốc độ tối đa. Cấu hình lệnh của ứng dụng có thể khác nhau; cần thiết lập app gửi đúng các ký tự trên.

## Điều khiển remote IR

Mã mặc định trong chương trình được dự kiến cho một số remote mini 21 phím:

- Phím `1`: Bluetooth
- Phím `2`: IR
- Phím `3`: tự động
- Mũi tên lên / xuống: tiến / lùi
- Mũi tên trái / phải: rẽ
- `OK`: dừng ở chế độ IR; trong chế độ tự động, chuyển về IR và dừng

Nếu remote không hoạt động, mở Serial Monitor ở `9600 baud`, bấm các phím để xem mã `command`, rồi cập nhật các hằng `IR_*` trong chương trình. Remote khác nhau có thể gửi mã khác.

## Tùy chỉnh thông số

Trong file `.ino`, có thể điều chỉnh:

- `KHOANG_CACH_NE_CM`: khoảng cách kích hoạt tránh vật cản ở chế độ tự động.
- `KHOANG_CACH_DUNG_CM`: khoảng cách dừng an toàn khi điều khiển tay.
- `TOC_DO_TU_DONG`: tốc độ chạy ở chế độ tự động (0–255).
- `THOI_GIAN_CHO_IR_MS`: thời gian chờ trước khi dừng sau khi nhả nút IR.
- Các mã `IR_*`: mã phím remote.

## Lưu ý an toàn và xử lý sự cố

- Cấp nguồn động cơ theo thông số của động cơ và Shield; không cấp động cơ từ chân 5V Arduino nếu nguồn không đủ dòng.
- Kiểm tra jumper/cách cấp nguồn của Shield L293D theo đúng phiên bản phần cứng. Không để nguồn động cơ vượt giới hạn cho phép của Shield.
- Nếu động cơ quay ngược, tắt nguồn rồi đảo hai dây động cơ tại cổng M3 hoặc M4 tương ứng.
- Nếu xe không nhận Bluetooth, kiểm tra TX/RX nối chéo, GND chung, baud rate `9600` và mức logic ở chân RX HC-05.
- Nếu HC-SR04 không phản hồi, chương trình sẽ dừng xe trong chế độ tự động hoặc khi xe đang tiến ở chế độ tay. Kiểm tra nguồn, GND, Trig và Echo.
- Thử nghiệm ở tốc độ thấp, khu vực trống và luôn sẵn sàng ngắt nguồn. Chương trình không thay thế nút ngắt nguồn vật lý.

## Cấu trúc kho mã đề xuất

```text
xe-thong-minh-arduino/
├── README.md
└── xe_thong_minh/
    └── xe_thong_minh.ino
```

