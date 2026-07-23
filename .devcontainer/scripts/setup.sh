#   Copyright (c) 2026 Juan Minor
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

<<Banner
    GNU C Compiler (gcc) Docker Development Image

    Developer: M., Juan

    Synopsis:

        This image is designed to provide a development environment for C programming using the GNU C Compiler (gcc). It includes essential tools and libraries for building and debugging C applications.
Banner

set -e

USERNAME="atom"

# @system
system() {
    apt-get update
    apt-get install -y zsh
    wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

    # Change shell
    chsh -s /bin/zsh "$USERNAME"

    # Install zsh plugins
    cp /root/.zshrc /home/"$USERNAME"/.zshrc
    cp -r /root/.oh-my-zsh /home/"$USERNAME"/.oh-my-zsh

    # Change ownership of the .zshrc and .oh-my-zsh directories
    chown -R "$USERNAME":"$USERNAME" /home/"$USERNAME"/.zshrc
    chown -R "$USERNAME":"$USERNAME" /home/"$USERNAME"/.oh-my-zsh
}

# @agy
install_agy() {
    curl -fsSL https://antigravity.google/cli/install.sh | zsh
    mkdir -p /home/"$USERNAME"/.local/bin
    cp -r /root/.local/bin/agy /home/"$USERNAME"/.local/bin/agy
    echo "export PATH=\"/home/$USERNAME/.local/bin:$PATH\"" >> /home/"$USERNAME"/.zshrc
    chown -R "$USERNAME":"$USERNAME" /home/"$USERNAME"/.local/bin
}

# @gdb
install_gdb() {
    apt-get install -y gdb
}

# gtest & lcov
install_gtest() {
    apt-get install -y libgtest-dev cmake lcov
    cd /usr/src/gtest
    cmake .
    make
    cp lib/*.a /usr/lib
}

install_libs() {
    apt-get install -y ripgrep
}

# Call the gtest installation function
system
install_agy
install_gdb
install_gtest
install_libs
