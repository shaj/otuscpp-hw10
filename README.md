# otuscpp-hw10



### Примеры команд
```
seq 100000 900000 | tr -d '\n' | ./bulkmt 1 | grep -v ^bulk
rm ./bulk*.log; seq 100000 900000 | ./bulkmt 30
rm ./bulk*.log; for i in $(seq 1 16); do sleep 2; echo $i; done | ./bulkmt 2; grep . bulk*.log | sort
rm ./bulk*.log; for i in $(seq 1 256); do sleep 2; echo $i; done | ./bulkmt 2 | ./bulkmt 2 | ./bulkmt 2; grep . bulk*.log | sort 
```
Просмотр логов
```
grep . bulk153*.log | sort
```
