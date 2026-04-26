FROM debian:bookworm
ENV DEBIAN_FRONTEND=noninteractive

# Essential packages for Qt5 Multimedia, OpenCV, and GStreamer Backend
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libopencv-dev \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libv4l-dev \
    x11-apps \
    qtbase5-dev \
    qtmultimedia5-dev \
    libqt5multimedia5 \
    libqt5multimedia5-plugins \
    libqt5multimediawidgets5 \
    # Added GStreamer plugins to fix the 'camerabin' and 'bad' errors
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-tools \
    libgstreamer-plugins-base1.0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
WORKDIR /app/df

# Run qmake and point it to the headers explicitly
RUN qmake . "INCLUDEPATH += /usr/include/qt5/QtMultimedia" && make -j$(nproc)

CMD ["./CPProject2"]