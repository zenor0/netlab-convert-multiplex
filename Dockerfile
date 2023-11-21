FROM ubuntu:latest 
WORKDIR /server

COPY ./server .
CMD [ "./server", "12345" ]
EXPOSE 12345

