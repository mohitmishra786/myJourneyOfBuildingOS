CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
LDFLAGS = -pthread

TARGETS = race_condition_demo producer_consumer_demo dining_philosophers_demo

all: $(TARGETS)

race_condition_demo: race_condition_demo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

producer_consumer_demo: producer_consumer_demo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

dining_philosophers_demo: dining_philosophers_demo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

run-race: race_condition_demo
	./race_condition_demo

run-producer-consumer: producer_consumer_demo
	./producer_consumer_demo

run-philosophers: dining_philosophers_demo
	./dining_philosophers_demo

run-all: all
	@echo "Running Race Condition Demo:"
	@./race_condition_demo
	@echo ""
	@echo "Running Producer-Consumer Demo:"
	@./producer_consumer_demo
	@echo ""
	@echo "Running Dining Philosophers Demo:"
	@./dining_philosophers_demo

.PHONY: all clean run-race run-producer-consumer run-philosophers run-all 