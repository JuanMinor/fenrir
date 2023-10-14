<<Banner
    GNU C Compiler (GCC) Docker Development Image

    Developer: Minor, J.
    
    Synopsis:
        Installs dependencies for GCC, zsh
Banner

set -e

root() {
    if [ "$(id -u)" -ne 0 ]; then
        echo -e "Setup script must be run with root privileges. Please try again."
        exit 1
    fi
}

system() {
    apt-get update
    apt-get install -y zsh
    wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

    # Change shell
    chsh -s /bin/zsh root
}

# @gdb
install_gdb() {
    apt-get install -y gdb
}

root
system
install_gdb
