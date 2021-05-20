FROM centos:8

# Important! To optimize docker layer size and reuse, try to keep the instructions and
# their order in similar images as much as possible. 

# Install OpenJDK 11. Reduce size by removing unnecessary stuff and optimize image size
RUN curl -O# https://download.oracle.com/java/GA/jdk11/9/GPL/openjdk-11.0.2_linux-x64_bin.tar.gz ;\
    tar zxvf openjdk-11.0.2_linux-x64_bin.tar.gz ;\
    mv jdk-11.0.2 /usr/local/ ; \
    ln -s /usr/local/jdk-11.0.2/bin/java /usr/bin/java ; \
    rm /openjdk-11.0.2_linux-x64_bin.tar.gz; \
    rm /usr/local/jdk-11.0.2/lib/src.zip; 
ENV JAVA_HOME="/usr/local/jdk-11.0.2"
ENV PATH=$PATH:$JAVA_HOME/bin

# Common utilities and clean
RUN echo 'alias ll="ls -l"' >> ~/.bashrc; \
    yum install -y which wget policycoreutils unzip nano initscripts openssl sudo; \
    yum clean all; rm -rf /var/cache/yum;

# JQ - Json parser
RUN curl -LO# https://github.com/stedolan/jq/releases/download/jq-1.6/jq-linux64; \
    mv ./jq-linux64 ./jq; \
    chmod +x ./jq; \
    mv jq /usr/bin

RUN echo "Downloading UKC client install file: $UKC_CLIENT_INSTALLER_URL.";

COPY ./data/ /root/data

RUN  rpm -i /root/data/ekm-client-2.0.2010.38445-el7+el8.x86_64.rpm; \
    rm $(basename /root/data/ekm-client-2.0.2010.38445-el7+el8.x86_64.rpm); \
    echo "UKC Client Installed successfully"

RUN echo "servers=ukc-ep:443">/etc/ekm/client.conf

ENV UKC_PARTITION encrypt_demo
ENV UKC_PASSWORD Unbound1!
ENV CLASSPATH /usr/lib64/

RUN chmod +x /root/data/*.sh;