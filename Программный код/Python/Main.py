import os
import pandas as pd
import time
import asyncio
import serial

com = input("Введите адрес последовательного порта: ")

#настройка последовательного порта
ser = serial.Serial(com, 9600, timeout = 1)
ser.flush()

#чтение таблицы и создание датафрейма
data = pd.read_excel('users.xlsx', usecols=['UID', 'Окончание доступа'])
df = pd.DataFrame(data)
print(df)

date_now = time.strftime('%Y-%m-%d', time.localtime()) #получаем текущую дату

#форматирование текущей даты и даты в таблице
date_now = pd.to_datetime(date_now)
df['Окончание доступа'] = pd.to_datetime(df['Окончание доступа'])

#отправка результата сравнения
async def Output(result):
        ser.write(result.encode() + b'\n')

async def checker(uid):
    #сравнение данных
    if uid in df['UID'].values:
        print('correct uid')
        usr_column = df.index[df['UID'] == uid]
        print('str: ', usr_column)
        if df.loc[usr_column, 'Окончание доступа'].iloc[0] > date_now:
            print('the pass is active')
            await Output('allow')
        else:
            print(df.loc[usr_column, 'UID'], df.loc[usr_column, 'Окончание доступа'])
            print(f'the pass has expired')
            await Output('expired')
    else:
        print(f'wrong uid: {uid}')
        await Output('forbid')

#получение данных и их обработка
async def Input():
    while True:
        if ser.in_waiting > 0:
            uid = ser.readline().decode('utf-8').rstrip()
            #форматирование полученного uid и uid в датафрейме
            uid = pd.to_numeric(uid)
            df['UID'] = pd.to_numeric(df['UID'])
            #отправка значений
            print(uid)
            await checker(uid)

#запуск функции
asyncio.run(Input())
