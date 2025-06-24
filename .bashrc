cd /root/mnt/dxvk/

alias brf32='cd /root/mnt/dxvk/ && rm -rf bld.w32 && meson setup --cross-file "build-win32.txt" --buildtype "release" --strip --prefix /root/mnt/dxvk/bld.w32/out/ bld.w32 && cd bld.w32 && ninja install && cp /root/mnt/dxvk/bld.w32/out/bin/d3d9.dll /root/mnt/dxvk/bld.w32/out/bin/d3d9_dxvk.dll'
alias w32='cd /root/mnt/dxvk/bld.w32'
alias br32='cd /root/mnt/dxvk/bld.w32 && ninja install && cp /root/mnt/dxvk/bld.w32/out/bin/d3d9.dll /root/mnt/dxvk/bld.w32/out/bin/d3d9_dxvk.dll'

alias bdf32='cd /root/mnt/dxvk/ && rm -rf bld.w32d && meson setup --cross-file "build-win32.txt" --buildtype "debug" --dev-build --prefix /root/mnt/dxvk/bld.w32d/out/ bld.w32d && cd bld.w32d && ninja install && cp /root/mnt/dxvk/bld.w32d/out/bin/d3d9.dll /root/mnt/dxvk/bld.w32d/out/bin/d3d9_dxvk.dll'
alias w32d='cd /root/mnt/dxvk/bld.w32d'
alias bd32='cd /root/mnt/dxvk/bld.w32d && ninja install && cp /root/mnt/dxvk/bld.w32d/out/bin/d3d9.dll /root/mnt/dxvk/bld.w32d/out/bin/d3d9_dxvk.dll'

alias brsf32='cd /root/mnt/dxvk/ && rm -rf bld.w32d && meson setup --cross-file "build-win32.txt" --buildtype "release" --prefix /root/mnt/dxvk/bld.w32d/out/ bld.w32d && cd bld.w32d && ninja install && cp /root/mnt/dxvk/bld.w32d/out/bin/d3d9.dll /root/mnt/dxvk/bld.w32d/out/bin/d3d9_dxvk.dll'

alias bupd='pacman-key --init && pacman -Sy --needed --noconfirm archlinux-keyring && pacman -Syu --needed --noconfirm clang meson glslang git mingw-w64 wine libunwind base bash base-devel sed tar curl wget gzip sudo file gawk grep bzip2 which pacman systemd findutils diffutils coreutils procps-ng util-linux xcb-util xcb-util-keysyms xcb-util-wm lib32-xcb-util lib32-xcb-util-keysyms glfw-x11'

