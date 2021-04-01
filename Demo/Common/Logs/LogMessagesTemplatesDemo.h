#pragma once

#include "../../../Threads/MessageLog.h"

using namespace Threader::Threads;

MESSAGE_TEMPLATE(1000, 0, Message,
                 "Запуск потока ожидания подключений на порт %d...");

MESSAGE_TEMPLATE(1001, 0, Warning,
                 "Завершение потока ожидания подключений на порт %d...");

MESSAGE_TEMPLATE(1002, 0, Message,
                 "Поток ожидания подключений на порт %d завершен");

MESSAGE_TEMPLATE(1003, 0, Message,
                 "Включение ожидания подключений на порт %d...");

MESSAGE_TEMPLATE(1004, 0, Message,
                 "Ожидание подключений на порт %d включено");

MESSAGE_TEMPLATE(1005, 0, Error,
                 "Ошибка включения ожидания подключений на порт %d - %d: %s");

MESSAGE_TEMPLATE(1006, 0, Warning,
                 "Запрос подключения на порт %d с адреса %s принят");


MESSAGE_TEMPLATE(1100, 0, Message,
                 "Запуск потока работы с подключением [%s]");

MESSAGE_TEMPLATE(1101, 0, Warning,
                 "Завершение потока работы с подключением [%s]...");

MESSAGE_TEMPLATE(1102, 0, Message,
                 "Поток работы с подключением [%s] завершен");

MESSAGE_TEMPLATE(1103, 0, Message,
                 "Выполняется подключение к [%s]...");

MESSAGE_TEMPLATE(1104, 0, Message,
                 "Подключение к [%s] установлено");

MESSAGE_TEMPLATE(1105, 0, Warning,
                 "Подключение к [%s] закрыто");

MESSAGE_TEMPLATE(1106, 0, Error,
                 "Ошибка работы с подключением к [%s] - %d: %s");

MESSAGE_TEMPLATE(1107, 0, Message,
                 "Получено %d байт от подключения к [%s]");

MESSAGE_TEMPLATE(1108, 0, Message,
                 "Передано %d байт на подключение к [%s]");

MESSAGE_TEMPLATE(1109, 0, Message,
                 "Получен пакет [%s] тип [%d], Id [%d], объем данных [%d] байт от подключения к [%s]");

MESSAGE_TEMPLATE(1110, 0, Message,
                 "Передан пакет [%s] тип [%d], Id [%d], объем данных [%d] байт от подключения к [%s]");

MESSAGE_TEMPLATE(1200, 0, Warning,
                 "Запрос авторизации клиента [%s] от подключения к [%s]");

MESSAGE_TEMPLATE(1201, 0, Warning,
                 "Запрос авторизации принят сервером [%s] от подключения к [%s]");

MESSAGE_TEMPLATE(1202, 0, Warning,
                 "Закрытие подключения к [%s]. Превышение времени ожидания активности [%d] мсек.");


MESSAGE_TEMPLATE(2000, 0, Message,
                 "Запуск потока работы с устройством [%s]");

MESSAGE_TEMPLATE(2001, 0, Message,
                 "Завершение потока работы с устройством [%s]...");

MESSAGE_TEMPLATE(2002, 0, Message,
                 "Поток работы с устройством [%s] завершен");

MESSAGE_TEMPLATE(2003, 0, Warning,
                 "Попытка подключения к устройству [%s]...");

MESSAGE_TEMPLATE(2004, 0, Message,
                 "Подключение к устройству [%s] установлено");

MESSAGE_TEMPLATE(2005, 0, Error,
                 "Ошибка подключения к устройству [%s]: %d - %s");

MESSAGE_TEMPLATE(2006, 0, Message,
                 "Подключение к устройству [%s] закрыто");


MESSAGE_TEMPLATE(3000, 0, Message,
                 "Запуск потока работы с UDP сокетом [%s]");

MESSAGE_TEMPLATE(3001, 0, Message,
                 "Завершение потока работы с UDP сокетом [%s]...");

MESSAGE_TEMPLATE(3002, 0, Message,
                 "Поток работы с UDP сокетом [%s] завершен");

MESSAGE_TEMPLATE(3003, 0, Warning,
                 "Попытка подключения к UDP сокету [%s]...");

MESSAGE_TEMPLATE(3004, 0, Message,
                 "Подключение к UDP сокету [%s] установлено");

MESSAGE_TEMPLATE(3005, 0, Error,
                 "Ошибка подключения к UDP сокету [%s]: %d - %s");

MESSAGE_TEMPLATE(3006, 0, Message,
                 "Подключение к UDP сокету [%s] закрыто");

MESSAGE_TEMPLATE(3007, 0, Message,
                 "Получено %d байт из UDP сокета [%s]");

MESSAGE_TEMPLATE(3008, 0, Message,
                 "Передано %d байт в UDP сокет [%s]");

MESSAGE_TEMPLATE(3009, 0, Message,
                 "Получено из UDP сокета [%s]: [%s]");

