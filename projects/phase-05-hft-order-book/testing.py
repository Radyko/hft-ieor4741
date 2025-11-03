import matplotlib.pyplot as plt

order_sizes = [1000, 5000, 10000, 50000, 100000]
execution_times = [0.00160896, 0.00841125, 0.0177102, 0.106437, 0.237786]  # Based on a trial run

plt.plot(order_sizes, execution_times, marker='o', linestyle='-', color='b')
plt.xlabel("Number of Orders")
plt.ylabel("Execution Time (seconds)")
plt.title("HFT Order Book Performance")
plt.grid()
plt.show()