import sys
import serial

from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QVBoxLayout,
    QLabel,
    QSlider,
    QProgressBar
)
from PyQt6.QtCore import Qt


class MainWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("UART Slider")
        self.resize(400, 150)

        # UART
        self.ser = serial.Serial(
            port="COM11",      # Linux: /dev/ttyUSB0
            baudrate=115200,
            timeout=1
        )

        layout = QVBoxLayout()

        self.label = QLabel("0")

        self.progress = QProgressBar()
        self.progress.setRange(0, 100)

        self.slider = QSlider(Qt.Orientation.Horizontal)
        self.slider.setRange(0, 100)

        self.slider.valueChanged.connect(self.value_changed)

        layout.addWidget(self.label)
        layout.addWidget(self.progress)
        layout.addWidget(self.slider)

        self.setLayout(layout)

    def value_changed(self, value):
        self.label.setText(str(value))
        self.progress.setValue(value)

        # UART'a gönder
        #self.ser.write(bytes([value]))

    def closeEvent(self, event):
        if self.ser.is_open:
            self.ser.close()
        event.accept()


app = QApplication(sys.argv)

window = MainWindow()
window.show()

sys.exit(app.exec())