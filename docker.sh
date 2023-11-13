#!/bin/bash

# Container name
container="deploy-freertos"

# Tag Docker Image
tag="freertos"
# Project name
project="esp32-freeRTOS2"
# branch
branch="main"
# dir name
app="test"
# App to be compiled
macro="APP_TEST"
# App version
app_version="1.0.0"

app_path=""

echo "Building ${app} app.."

if [ -d "images/${app}" ]; then
    rm -r "images/${app}"
fi

file_name="esp32-freeRTOS2.ino"
if [ ! -e "$file_name" ]; then
    wget -q https://github.com/zimbora/${project}/archive/refs/heads/${branch}.zip
    unzip ${branch}.zip
    rm ${branch}.zip
    mv ${project}-${branch} ${project}
    cd ${project}

    if [ ! -d src/app/${app} ]; then
        mkdir -p src/app/${app}
        cp *.cpp *.c *.h *.hpp deploy.sh src/app/${app}
    fi
else
    echo "We are working on esp32-freeRTOS repository"

    # Specify the path to the symbolic link
    symbolic_link="src/app/${app}"

    # Check if the path exists
    if [ -e "$symbolic_link" ]; then
        echo "The path is a symbolic link: ${symbolic_link}"
        # Use readlink to get the actual path of the symbolic link
        app_path=$(readlink -n "$symbolic_link")
        echo "path for app: ${app_path}"
        mkdir -p $app
        # Use find to locate files and move them to the destination folder
        find "$app_path" -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" -o -name "deploy.sh" \) -exec cp {} "$app" \;
    fi
fi

docker build \
--build-arg DOCKERFILE_PATH=Dockerfile.local \
--build-arg PROJECT=$project \
--build-arg APP=$app \
--build-arg MACRO=$macro \
--build-arg APP_VERSION=$app_version \
--build-arg APP_LINK=$app_path \
-t ${tag} .

#docker run -it --name $container ${tag}:latest
#docker stop $container

docker create --name ${container} ${tag}
docker cp ${container}:${project}/images/${app} ./images/

#docker rm $container
#docker rmi $tag


if [ -d $app_path ]; then
    rm -r $app
fi

