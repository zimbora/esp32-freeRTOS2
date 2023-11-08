#!/bin/bash

# Container name
container="deploy-freertos"

# Tag Docker Image
tag="freertos"
# Project name
project="esp32-freeRTOS2"
# App to be compiled
app="APP_TEST"
# App version
app_version="1.0.0"

if [ -d "images/${app}" ]; then
    rm -r "images/${app}"
fi

docker rm $container

docker build \
--build-arg PROJECT=$project \
--build-arg APP=$app \
--build-arg APP_VERSION=$app_version \
-t ${tag} .

docker run -it --name $container ${tag}:latest

docker cp ${container}:${project}/images/${app} ./images/

docker stop $container
