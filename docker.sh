
# Container name
container="deploy-freertos"

# Tag Docker Image
tag="freertos"

# Project name
project="esp32-freeRTOS2"

# App to be compiled
app="DEMO"

docker rm $container

docker build \
--build-arg PROJECT=$project \
--build-arg APP=$app \
-t ${tag} .

docker run -it --name $container ${tag}:latest

docker cp ${container}:${project}/images/${app} ./images/

docker stop $container
