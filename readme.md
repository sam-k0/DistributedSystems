# Docker setup:

Ist mittlerweile sehr einfach mit der docker-compose.yaml

Alles was gemacht werden muss ist `docker-compose up -d` im directory mit der *.yml file

- *Falls es eine ! FEHLERMELDUNG ! gibt, dass ein image nicht gefunden werden kann, muss man 'y' eingeben, das image wird dann neu gebaut.*

- *Die Startreihenfolge wird in der YAML Datei schon festgelegt.*

- *Der* ***WEBSERVER*** *kann über Port 8080 erreicht werden: `localhost:8080` im Browser eingeben*

- *Für einen ***REBUILD*** kann der folgende Befehl verwendet werden, um die alten Images bei Änderungen neu zu bauen:* `docker-compose up --build --force-recreate -d`

# Weiteres:
`docker ps` -> Listet alle container auf
`docker network ls` -> Listet network auf
`docker build -t "myProgramm" .` -> Baut dockerfile im current directory

Wenn die Container nich im richtigen Network laufen (Bridge) kann man dies Explizit einstellen:
`docker run [image] . --net=bridge`

Wenn es dann immernoch nicht geht, sind die IPaddressen wahrscheinlich falsch:
Diese müssen dann in `config.h` im code geändert werden.

## Herausfinden der Dockercontainer IPaddr:

1. Docker container ID finden: `docker ps`
2. Docker inspect aufrufen und nach IPADDR filtern: `docker inspect [CONTAINER ID] | grep IPAddress`

Diese muss dann in den headern gesetzt werden.
Der MCU bekommt dabei die IPaddr des Sensor-containers und andersherum.