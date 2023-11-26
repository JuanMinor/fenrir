<<Banner
    GNU C Compiler (gcc) Docker Development Image

    Developer: Minor, J.
    
    Synopsis:
        Installs dependencies for gcc, zsh, gdb
Banner

set -e

system() {
    apt-get update
    apt-get install -y zsh
    wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

    # Change shell
    chsh -s /bin/zsh atom
}

# @gdb
install_gdb() {
    apt-get install -y gdb
}

system
install_gdb
