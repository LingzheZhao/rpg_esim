FROM osrf/ros:melodic-desktop-full

ARG DEBIAN_FRONTEND=noninteractive 
ARG PROJ_PATH=/home/user/sim_ws
ARG ESIM_PATH=$PROJ_PATH/src/rpg_esim
ENV TZ=Asia/Shanghai LANG=C.UTF-8 LC_ALL=C.UTF-8 PIP_NO_CACHE_DIR=1 PIP_CACHE_DIR=/tmp/

# Installing some essential system packages
RUN sed -i "s/archive.ubuntu.com/mirrors.ustc.edu.cn/g" /etc/apt/sources.list &&\
    sed -i "s/security.ubuntu.com/mirrors.ustc.edu.cn/g" /etc/apt/sources.list &&\
    apt-get update && apt-get upgrade -y &&\
    apt-get install -y --no-install-recommends \
      # Common
      autoconf automake autotools-dev build-essential ca-certificates \
      make cmake ninja-build pkg-config g++ ccache yasm \
      ccache doxygen graphviz plantuml \
      daemontools krb5-user ibverbs-providers libibverbs1 \
      libkrb5-dev librdmacm1 libssl-dev libtool \
      libnuma1 libnuma-dev libpmi2-0-dev \
      openssh-server openssh-client nfs-common \
      ## Tools
      git curl wget unzip nano vim-tiny net-tools sudo htop iotop iputils-ping \
      cloc rsync screen tmux xz-utils software-properties-common &&\
    wget "https://raw.githubusercontent.com/ros/rosdistro/master/ros.key" -O - | apt-key add - &&\
    # echo "deb https://mirrors.tuna.tsinghua.edu.cn/ros/ubuntu/ bionic main /" > \
    #     /etc/apt/sources.list.d/ros1-snapshots.list &&\
    apt-get install -y --no-install-recommends \
      ## Deps
      libassimp-dev \
      libfftw3-dev libfftw3-doc \
      libglew-dev \
      libglfw3-dev \
      libglm-dev \
      libopencv-dev \
      libproj-dev \
      libyaml-cpp-dev \
      python3-rosdep \
      python3-vcstool \
      python3-catkin-tools \
      python3-pip \
      ros-melodic-camera-info-manager \
      ros-melodic-cv-bridge \
      ros-melodic-eigen-conversions \
      ros-melodic-image-view \
      ros-melodic-image-geometry \
      ros-melodic-image-transport \
      ros-melodic-pcl-ros \
      ros-melodic-rviz \
      ros-melodic-sophus \
      ros-melodic-tf-conversions \
      ros-melodic-tf \
    && rm /etc/ssh/ssh_host_ecdsa_key \
    && rm /etc/ssh/ssh_host_ed25519_key \
    && rm /etc/ssh/ssh_host_rsa_key \
    && cp /etc/ssh/sshd_config /etc/ssh/sshd_config_bak \
    && sed -i "s/^.*X11Forwarding.*$/X11Forwarding yes/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11UseLocalhost.*$/X11UseLocalhost no/" /etc/ssh/sshd_config \
    && grep "^X11UseLocalhost" /etc/ssh/sshd_config || echo "X11UseLocalhost no" >> /etc/ssh/sshd_config \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add - &&\
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' &&\
    apt-get update &&\
    apt-get install -y cmake libzmq3-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --recursive https://github.com/zeromq/cppzmq.git &&\
    cd cppzmq &&\
    git checkout v4.10.0 &&\
    mkdir build &&\
    cd build &&\
    cmake -DCPPZMQ_BUILD_TESTS=OFF .. &&\
    make -j install &&\
    cd /tmp &&\
    rm -rf cppzmq

WORKDIR $PROJ_PATH
RUN pip3 install -U pip --no-cache-dir &&\
    catkin init &&\
    catkin config --extend /opt/ros/melodic --cmake-args -DCMAKE_BUILD_TYPE=Release &&\
    mkdir -p $ESIM_PATH

COPY ./ $ESIM_PATH
WORKDIR $PROJ_PATH/src
RUN vcs-import < $ESIM_PATH/dependencies.yaml

WORKDIR $PROJ_PATH/src/ze_oss
RUN touch \
	imp_3rdparty_cuda_toolkit/CATKIN_IGNORE \
       imp_app_pangolin_example/CATKIN_IGNORE \
       imp_benchmark_aligned_allocator/CATKIN_IGNORE \
       imp_bridge_pangolin/CATKIN_IGNORE \
       imp_cu_core/CATKIN_IGNORE \
       imp_cu_correspondence/CATKIN_IGNORE \
       imp_cu_imgproc/CATKIN_IGNORE \
       imp_ros_rof_denoising/CATKIN_IGNORE \
       imp_tools_cmd/CATKIN_IGNORE \
       ze_data_provider/CATKIN_IGNORE \
       ze_geometry/CATKIN_IGNORE \
       ze_imu/CATKIN_IGNORE \
       ze_trajectory_analysis/CATKIN_IGNORE

WORKDIR $PROJ_PATH
RUN catkin build esim_ros

RUN echo ". /opt/ros/melodic/setup.bash" >> /root/.bashrc &&\
    echo ". /home/user/sim_ws/devel/setup.bash" >> /root/.bashrc

ENTRYPOINT  [". /home/user/sim_ws/devel/setup.bash"]
