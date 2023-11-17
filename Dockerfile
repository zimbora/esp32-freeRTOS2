FROM zimbora/arduino-deploy:latest

# Set a default value (optional)
ENV BASE_DIR=/root
ARG PROJECT="esp32-freeRTOS2"
ARG APP="demo"
ARG MACRO="APP_DEMO"
ARG APP_LINK=""
ARG APP_VERSION

# Copy the script into the container
COPY ./${PROJECT} /${PROJECT}

# Make the script executable
#RUN chmod +x /${PROJECT}/src/app/demo/deploy.sh
RUN id && \
    ls -l /${PROJECT}/src/app/${APP} && \
    chmod +x /${PROJECT}/src/app/${APP}/deploy.sh

RUN /${PROJECT}/src/app/${APP}/deploy.sh -d $BASE_DIR -p $PROJECT -a $APP -m $MACRO -v $APP_VERSION --docker > build_output.txt 2>&1
