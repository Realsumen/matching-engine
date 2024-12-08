import socket
import json

# 配置
ADDRESS = "127.0.0.1"
PORT = 7001

def send_message_to_server(message_type, message_data):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            client_socket.connect((ADDRESS, PORT))
            print(f"Connected to {ADDRESS}:{PORT}")

            message = {"type": message_type}
            message.update(message_data)  # 合并消息数据
            message_str = json.dumps(message)

            client_socket.sendall(message_str.encode('utf-8'))
            print(f"Sent: {message_str}")

            response = client_socket.recv(1024)
            print(f"Response: {response.decode('utf-8')}")
    except Exception as e:
        print(f"Error: {e}")

def create_add_order_message(instrument, price, quantity, isBuy, orderType):
    return {
        "instrument": instrument,
        "price": price,
        "quantity": quantity,
        "isBuy": isBuy,
        "orderType": orderType
    }

def create_modify_order_message(orderId, instrument, newPrice, newQuantity):
    return {
        "orderId": orderId,
        "instrument": instrument,
        "newPrice": newPrice,
        "newQuantity": newQuantity
    }

def create_cancel_order_message(orderId, instrument):
    return {
        "orderId": orderId,
        "instrument": instrument
    }

if __name__ == "__main__":
    add_order_data = create_add_order_message("AAPL", 150.25, 100, True, "LIMIT")
    send_message_to_server("ADD_ORDER", add_order_data)

    # modify_order_data = create_modify_order_message(12345, "AAPL", 155.50, 150)
    # send_message_to_server("MODIFY_ORDER", modify_order_data)
    #
    # cancel_order_data = create_cancel_order_message(12345, "AAPL")
    # send_message_to_server("CANCEL_ORDER", cancel_order_data)




