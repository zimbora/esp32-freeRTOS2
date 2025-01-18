FROM zimbora/arduino-deploy-amd:latest

ENV BASE_DIR=/root
ARG PROJECT="esp32-freeRTOS2"
ARG APP="demo"
ARG APP_VERSION="1.0.0"

# Copy the script into the container
COPY . /${PROJECT}

# Make the script executable
RUN chmod +x /${PROJECT}/deploy.sh

# Set the working directory to your project directory
WORKDIR /${PROJECT}

RUN ./deploy.sh -d $BASE_DIR -p $PROJECT -a $APP -va $APP_VERSION
