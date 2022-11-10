import { WebSocketServer } from "ws";

const PORT = process.env.PORT || 3030;

const server = new WebSocketServer({ port: PORT });

let list = [];
const sockets = [];
let idCounter = 0;

//Manda em formato de json para o usuário
const sendJson = function (socket, data){
    socket.send(JSON.stringify(data));
}

//insere um novo item na lista
const insert = function(item) {
    item.id = idCounter;
    list.push(item);
    idCounter++;
}

//seta um item da lista como marcado ou desmarcado
const toggle = function(item, state){
  console.log({item});
    list = list.map((it) => it.id == item.id ? {...it, state} : it);
}

//remove um item da lista
const remove = function(item){
    list = list.filter((it) => it.id != item.id);
}

//manda a lista para todos os usuários conectados
const broadcastList = function(){
    console.log(list);
    sockets.forEach((socket) => {
        sendJson(socket, {type: "list", list});
    })
}

server.on("connection", (socket) => {
  sockets.push(socket);
  // send a message to the client
  sendJson(socket, {type: "list", list});

  // receive a message from the client
  socket.on("message", (data) => {
    console.log({data})
    const packet = JSON.parse(data);

    switch (packet.type) {
      case "insert":
        insert(packet.item);
        broadcastList();
        break;
      case "check":
        toggle(packet.item, true);
        broadcastList();
        break;
      case "uncheck":
        toggle(packet.item, false);
        broadcastList();
        break;
      case "remove":
        remove(packet.item);
        broadcastList();
        break;
    }
  });
});

console.log("Listening web socket on port " + PORT);