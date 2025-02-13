#!/bin/bash

# Container name
container="deploy-freertos"
# Tag Docker Image
tag="freertos"
# Project name
project="esp32-freeRTOS2"
# dir name
app="demo"
# App version
app_version="1.0.0"
# folder to store generated images
folder="images"

echo "Building ${app} app.."

if [ -d $folder ]; then
  rm -r $folder
fi

docker build --no-cache \
-f Dockerfile.local \
--build-arg PROJECT=$project \
--build-arg APP=$app \
--build-arg APP_VERSION=$app_version \
-t ${tag} .

docker run --name $container ${tag}:latest
#debug
#docker run -it --name $container ${tag}:latest

docker cp ${container}:${project}/${folder}/ ./${folder}/
sha256sum ${folder}/${project}.ino.merged.bin

docker stop $container

docker rm $container
docker rmi $tag
