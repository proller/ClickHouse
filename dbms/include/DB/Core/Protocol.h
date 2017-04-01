#pragma once

#include <DB/Core/Types.h>


namespace DB
{

/** Протокол взаимодействия с сервером.
  *
  * Клиент соединяется с сервером и передаёт ему пакет Hello.
  * Если версия не устраивает, то сервер может разорвать соединение.
  * Сервер отвечает пакетом Hello.
  * Если версия не устраивает, то клиент может разорвать соединение.
  *
  * Далее в цикле.
  *
  * 1. Клиент отправляет на сервер пакет Query.
  *
  * Начиная с версии 50263, сразу после отправки пакета Query клиент начинает передачу
  *  внешних (временных) таблиц (external storages) - один или несколько пакетов Data.
  * Конец передачи данных определается по отправленному пустому блоку.
  * В данный момент, не пустое множество таблиц может быть передано только вместе с запросом SELECT.
  *
  * Если запрос типа INSERT (требует передачи данных от клиента), то сервер передаёт
  *  пакет Data, содержащий пустой блок, который описывает структуру таблицы.
  * Затем клиент отправляет данные для вставки
  * - один или несколько пакетов Data.
  * Конец данных определается по отправленному пустому блоку.
  * Затем сервер отправляет клиенту пакет EndOfStream.
  *
  * Если запрос типа SELECT или другой, то сервер передаёт набор пакетов одного из следующих видов:
  * - Data - данные результата выполнения запроса (один блок);
  * - Progress - прогресс выполнения запроса;
  * - Exception - ошибка;
  * - EndOfStream - конец передачи данных;
  *
  * Клиент должен читать пакеты до EndOfStream или Exception.
  * Также, клиент может передать на сервер пакет Cancel - отмена выполнения запроса.
  * В этом случае, сервер может прервать выполнение запроса и вернуть неполные данные;
  *  но клиент всё равно должен читать все пакеты до EndOfStream.
  *
  * Перед пакетом EndOfStream, если есть профайлинговая информация и ревизия клиента достаточно новая,
  * может быть отправлен пакет Totals и/или ProfileInfo.
  * Totals - блок с тотальными значениями.
  * ProfileInfo - данные профайлинга - сериализованная структура BlockStreamProfileInfo.
  *
  * При запросах, которые возвращают данные, сервер, перед обработкой запроса,
  * отправляет заголовочный блок, содержащий описание столбцов из запроса, но с нулем строк.
  * Используя этот заголовочный блок, клиент может заранее проинициализировать формат вывода
  * и вывести префикс таблицы результата.
  *
  * 2. Между запросами, клиент может отправить Ping, и сервер должен ответить Pong.
  */

namespace Protocol
{
    /// То, что передаёт сервер.
    namespace Server
    {
        enum Enum
        {
            Hello = 0,            /// Имя, версия, ревизия.
            Data = 1,            /// Блок данных со сжатием или без.
            Exception = 2,        /// Исключение во время обработки запроса.
            Progress = 3,        /// Прогресс выполнения запроса: строк считано, байт считано.
            Pong = 4,            /// Ответ на Ping.
            EndOfStream = 5,    /// Все пакеты были переданы.
            ProfileInfo = 6,    /// Пакет с профайлинговой информацией.
            Totals = 7,            /// Блок данных с тотальными значениями, со сжатием или без.
            Extremes = 8,        /// Блок данных с минимумами и максимумами, аналогично.
        };

        /** NOTE: Если бы в качестве типа агрумента функции был бы Enum, то сравнение packet >= 0 && packet < 7
          * срабатывало бы всегда из-за оптимизации компилятором, даже если packet некорректный, и было бы чтение за границей массива.
          * https://www.securecoding.cert.org/confluence/display/cplusplus/INT36-CPP.+Do+not+use+out-of-range+enumeration+values
          */
        inline const char * toString(UInt64 packet)
        {
            static const char * data[] = { "Hello", "Data", "Exception", "Progress", "Pong", "EndOfStream", "ProfileInfo", "Totals", "Extremes" };
            return packet < 9
                ? data[packet]
                : "Unknown packet";
        }
    }

    /// То, что передаёт клиент.
    namespace Client
    {
        enum Enum
        {
            Hello = 0,            /// Имя, версия, ревизия, БД по-умолчанию.
            Query = 1,            /** Идентификатор запроса, настройки на отдельный запрос,
                                  * информация, до какой стадии исполнять запрос,
                                  * использовать ли сжатие, текст запроса (без данных для INSERT-а).
                                  */
            Data = 2,            /// Блок данных со сжатием или без.
            Cancel = 3,            /// Отменить выполнение запроса.
            Ping = 4,            /// Проверка живости соединения с сервером.
        };

        inline const char * toString(UInt64 packet)
        {
            static const char * data[] = { "Hello", "Query", "Data", "Cancel", "Ping" };
            return packet < 5
                ? data[packet]
                : "Unknown packet";
        }
    }

    /// Использовать ли сжатие.
    namespace Compression
    {
        enum Enum
        {
            Disable = 0,
            Enable = 1,
        };
    }
}

}
