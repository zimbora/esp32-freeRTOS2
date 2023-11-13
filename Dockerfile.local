# Use a base image that has bash and apt (e.g., ubuntu)
FROM zimbora/arduino-deploy:latest

# Set a default value (optional)
ENV BASE_DIR=/root
ARG PROJECT="esp32-freeRTOS2"
ARG APP="demo"
ARG MACRO="APP_DEMO"
ARG APP_LINK=""
ARG APP_VERSION

RUN echo "APP:${APP}"

# Copy the script into the container
COPY . ${PROJECT}

# Check if app is referred by a link
RUN if [ -z "$APP_LINK" ] ; then \
		echo No link provided for app source ; \
	else \
		cd ${PROJECT}; \
		rm -rf src/app; \
		mkdir -p src/app; \
		mv ${APP} src/app/; \
		cd .. ; \
	fi > build_setup.txt 2>&1

# Make the script executable
RUN chmod +x ${PROJECT}/src/app/${APP}/deploy.sh

RUN ${PROJECT}/src/app/${APP}/deploy.sh -d $BASE_DIR -p $PROJECT -a $APP -m $MACRO -v $APP_VERSION --docker > build_output.txt 2>&1
