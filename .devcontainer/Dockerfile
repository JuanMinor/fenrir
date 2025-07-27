FROM gcc:latest

# @atom
ARG UID=1000
ARG USERNAME="atom"

# ENV
ENV CI="false"

RUN groupadd --gid $UID $USERNAME \
	&& useradd --uid $UID --gid $UID -m $USERNAME \
	&& apt-get update \
	&& apt-get install -y sudo wget \
	&& echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
	&& chmod 0440 /etc/sudoers.d/$USERNAME

COPY --chown=atom:atom ./scripts/setup.sh /tmp/setup.sh

RUN chmod +x /tmp/setup.sh ./tmp/setup.sh

RUN /tmp/setup.sh

USER atom
