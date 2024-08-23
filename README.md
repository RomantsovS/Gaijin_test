Тестовое задание программист С++
Нужно написать два приложения, клиент и сервер, которые взаимодействуют друг с другом.

Сервер

У сервера есть конфигурационный файл, он лежит на диске, config.txt, в нем хранитятся данные
ключ/значение, можно использовать произвольный формат. На старте сервер его читает. У сервера
есть 2 команды get и set, первая по ключу получает значение, вторая его записывает. Формат
команд $get <key>, $set <key>=<value>. Запись значения должна сопровождаться обновлением
файла на диске, можно не сразу, а периодически. Желательно вести запись в файл в отдельном
потоке. Сервер поддерживает произвольное колличество клиентов. Сервер должен быть
многопоточным, т.е. команды должны обрабатываться параллельно. Чтение должно происходить с
минимальной задержкой. Считаем что запись редкая ситуация. Сервер должен вести статистику
запросов, и выводить в консоль каждый 5 секунд сколько каких запросов он выполнил всего и за
последние 5 секунд. Опционально – считать статистику доступа к каждому ключу, возвращать на
любую из команд в результате эту статистику, например:
$get tree
результат:
tree=Blue
reads=10
writes=1
При разработке можно использовать сторонние библиотеки, для парсинга и для сети. Например
rapidjson, boost и т.д.

Клиент

Однопоточный. Подключается к серверу, далее выбирает случайный ключ из захардкоженого
списка и выполняет на сервере $get вероятность 99%, а в 1% случаев для записывает случайны
данные в этот ключ, выполняя $set. Результаты выполнения команд пишутся в консоль. Это
повторяется в цикле 10к раз(без разрыва соединения) и происходит выход. Опционально сделать
реконнект к серверу, в случае разрыва соединения, или если сервер офлайн, т.е. клиент ждет пока
он появится в сети.
Клиент можно написать на любом языке, хоть на питоне.
