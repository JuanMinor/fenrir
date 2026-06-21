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

	# Install zsh plugins
	cp /root/.zshrc /home/atom/.zshrc
	cp -r /root/.oh-my-zsh /home/atom/.oh-my-zsh

	# Change ownership of the .zshrc and .oh-my-zsh directories
	chown -R atom:atom /home/atom/.zshrc
	chown -R atom:atom /home/atom/.oh-my-zsh
}

# @fonts
install_fonts() {
	apt-get install -y fonts-jetbrains-mono
	fc-cache -f -v
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
install_fonts
install_gdb
install_gtest
install_libs
