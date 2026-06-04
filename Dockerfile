FROM zimbora/arduino-deploy-amd:latest

ENV BASE_DIR=/root
ARG PROJECT="esp32-freeRTOS2-c5"
ARG APP="demo"
ARG BUILD="dev"
ARG SKETCH="esp32-freeRTOS2"

# Copy the source into a directory matching the sketch name (arduino-cli requirement)
COPY . /${SKETCH}

# Make the script executable
RUN chmod +x /${SKETCH}/deploy.sh

# Set the working directory to the sketch directory
WORKDIR /${SKETCH}

RUN ./deploy.sh -d $BASE_DIR -p $PROJECT -a $APP -b $BUILD
