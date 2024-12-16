/*
Разработана серверная часть.
Base URL - http://192.168.0.26:1322/ (работает внутри нашей сети)
Авторизация пользователя:
URL - http://192.168.0.26:1322/api/auth/getToken (post, без авторизации)
В тело запроса отдаем json в таком формате
{
    "Login": "test",
    "Password": "test"
}
В ответ приходит json в формате
{
    "login":"test",
    "token":""
}
После этого в каждый запрос в header прикладываем jwt token, полученный выше
Запрос на получение номеров карт сотрудников, которые могут открывать ящик:
URL - http://192.168.0.26:1322/api/card/ (get, авторизация нужна)
В тело запроса прикладываем mac адрес в таком формате
{
    "mac"
}
В ответ получаем массив адресов в таком формате
[
{"number1"},
{"number2"},
{"number3"}
]
Запрос для записи действия пользователя:
URL - http://192.168.0.26:1322/api/log/ (post, авторизация нужна)
В тело запроса поместить данные в следующем формате:
{
    "Card": "number",
    "Mac": "mac",
    "Cell": "cell",
    "Date": "01.01.2000",
    "isHere": true
}
В ответ получим statuscode 200 - значит всё записано

Начало добавления
{
"command":"start",
"value":""
}
команда на считывание датчиков при отсутствии инструмента

Окончание добавления
{
"command":"end",
"value":"1"
}
команда на считывание профиля с добавленным инструментом и присвоение ему ID номера


Ответ от контроллера при добавлении
{
"command":"ok",
"value":"1"
}
Ответ с привязанным ID оборудования

Подсветка
{
"command":"light",
"value":"1"
}
light - включение подсветки, связанной с данным инструментом, 
value - id инструмента, который нужно выдать...


{"command":"start","value":""}
*/