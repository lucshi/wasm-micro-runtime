
1. How to build iwasm in Linux

  cd products/linux/
  mkdir build && cd build
  cmake .. && make

2. How to build iwasm in Zephyr system
  (1) clone zephyr source code

    git clone https://github.com/zephyrproject-rtos/zephyr.git

  (2) copy iwasm products/zephyr/sample directory to zephry/samples/

    cd zephyr/samples/
    cp -a <iwasm_root_dir>/products/zephyr/sample .
    cd sample

  (3) create a link to iwasm root directory and rename it to iwasm

    ln -s <iwasm_root_dir> iwasm

  (4) build source code

    mkdir build && cd build
    source ../../../zephyr-env.sh
    cmake -GNinja -DBOARD=qemu_x86_nommu ..         => qemu_x86 isn't supported
    ninja

  (5) run sample

    ninja run

3. How to build iwasm in AliOS-Things platform
  (1) clone AliOS-Things source code

    git clone https://github.com/alibaba/AliOS-Things.git

  (2) copy iwasm products/alios-things directory to AliOS-Things/middleware,
      and rename it as iwasm

    cp -a <iwasm_root_dir>/products/alios-things middleware/

  (3) create a link to iwasm root directory and rename it to iwasm

    ln -s <iwasm_root_dir> middleware/iwasm/iwasm

  (4) modify sample source code of AliOS-Things to call iwasm_init() function

    modify file app/example/helloworld/helloworld.c, in the beginning of
    function application_start(), add:

      bool iwasm_init(void);
      iwasm_init();

    modify file app/example/helloworld/helloworld.mk, change
      $(NAME)_COMPONENTS := yloop cli
    to
      $(NAME)_COMPONENTS := yloop cli iwasm

  (4) build source code

    aos make helloworld@linuxhost

  (5) run source code
    ./out/helloworld@linuxhost/binary/helloworld@linuxhost.elf
