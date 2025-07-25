#! /usr/bin/env bash

success=yes

. "scripts/test_funcs.sh"

if [[ "$#" -ne "1" ]]; then
  >&2 echo "Usage: ./server_arithmetic.sh <port>"
  exit 1
fi

port="$1"

ensure_supervised

>&2 echo "Starting server"
start_server ${port}

stem=$(basename "$0" .sh)
mkdir -p actual

fifo_1="${stem}_fifo_1.fifo"
fifo_2="${stem}_fifo_2.fifo"
mkfifo "$fifo_1"
mkfifo "$fifo_2"

>&2 echo "Starting reference clients..."
./scripts/ref_client.rb localhost "$port" '--' < "$fifo_1" > actual/${stem}_1.out &
CLIENT_1_PID=$!
./scripts/ref_client.rb localhost "$port" '--' < "$fifo_2" > actual/${stem}_2.out &
CLIENT_2_PID=$!

exec 7> "$fifo_1"
exec 8> "$fifo_2"

>&2 echo "Sending arithmetic test requests..."
>&7 echo "LOGIN alice"
>&7 echo "CREATE nums"
>&7 echo "PUSH 5"
>&7 echo "PUSH 3"
>&7 echo "ADD"
>&7 echo "TOP"        # 8
>&7 echo "PUSH 10"
>&7 echo "SUB"
>&7 echo "TOP"        # -2
>&7 echo "PUSH -4"
>&7 echo "MUL"
>&7 echo "TOP"        # 8
>&7 echo "PUSH 2"
>&7 echo "DIV"
>&7 echo "TOP"        # 4
>&7 echo "SET nums result"
>&7 echo "GET nums result"
>&7 echo "TOP"
>&7 echo "BYE"

>&8 echo "LOGIN bob"
>&8 echo "GET nums result"
>&8 echo "TOP"
>&8 echo "PUSH 0"
>&8 echo "DIV"         # should fail (divide by zero)
>&8 echo "PUSH abc"
>&8 echo "PUSH 2"
>&8 echo "ADD"         # should fail (non-integer)
>&8 echo "BYE"

sleep 1

exec 7>&-
exec 8>&-

wait $CLIENT_1_PID
wait $CLIENT_2_PID

rm -f "$fifo_1" "$fifo_2"

kill -TERM $SERVER_PID
sleep 1

>&2 echo "Comparing expected outputs against actual outputs..."
diff_output "expected/${stem}_1.out" "actual/${stem}_1.out"
diff_output "expected/${stem}_2.out" "actual/${stem}_2.out"

if [[ "$success" = "yes" ]]; then
  >&2 echo "Success!"
  exit 0
fi

exit 1