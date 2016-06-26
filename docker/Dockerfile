FROM gcc:5.3
MAINTAINER Philippe Ferdinand <pshampanier@gmail.com>

# Adding postgresql apt repository
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ jessie-pgdg main" > /etc/apt/sources.list.d/pgdg.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add -

RUN apt-get update 

# Adding gdb & doxygen
RUN apt-get install -y gdb  
RUN apt-get install -y doxygen

# Installing cmake
ENV CMAKE_INSTALL_SCRIPT=cmake-3.5.1-Linux-x86_64.sh
RUN curl -s https://cmake.org/files/v3.5/$CMAKE_INSTALL_SCRIPT -o /tmp/$CMAKE_INSTALL_SCRIPT
RUN chmod +x /tmp/$CMAKE_INSTALL_SCRIPT
RUN /tmp/$CMAKE_INSTALL_SCRIPT --skip-license
RUN rm /tmp/$CMAKE_INSTALL_SCRIPT

# Add the build user
RUN useradd build --create-home

# Configure sudo for the build user
RUN apt-get -y install sudo && \
    echo "build:changeme" | chpasswd && \
    adduser build sudo
  
#
# Installing postgres
#
# make the "en_US.UTF-8" locale so postgres will be utf-8 enabled by default
RUN apt-get update && apt-get install -y locales && \
  localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8 && \
  update-locale LANG=en_US.UTF-8
ENV LANG en_US.UTF8
ENV LC_ALL en_US.UTF8
ENV LANGUAGE en_US.UTF8
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y postgresql-9.5 postgresql-contrib-9.5 postgresql-client-9.5
RUN apt-get install -y postgresql-server-dev-9.5

# Cleanup
RUN apt-get clean all

# Finalize the postgres install
COPY pg_hba.conf /etc/postgresql/9.5/main/pg_hba.conf
USER postgres
RUN /etc/init.d/postgresql start && \
  createuser ci-test && \
  createdb ci-test --owner=ci-test --encoding=UTF8

USER build
VOLUME /home/build
WORKDIR /home/build

# Set the default command to run when starting the container
# USER postgres
# CMD ["/usr/lib/postgresql/9.5/bin/postgres", "-D", "/var/lib/postgresql/9.5/main/", "-c", "config_file=/etc/postgresql/9.5/main/postgresql.conf"]
