
git clone https://github.com/zephyrproject-rtos/zephyr.git

cd zephyr/samples/

git clone ssg_micro_runtime-wasm-poc

cd ssg_micro_runtime-wasm-poc/zephyr_sample

mkdir build && cd build

source ../../../../zephyr-env.sh

cmake -GNinja -DBOARD=qemu_x86_nommu ..         => qemu_x86 isn't supported

ninja

ninja run
