My strategy for the momentum was to track when the pervious three prices were monotonely increasing or decreasing. This was done by using a deque and when its size was greater than or equal to three, removing the front (the price that has been on the longest) making space for a new price. This way, the deque always had the three most recent prices.

With those prices, I could calculate if they were increasing or decreasing by checking if price[0] > (<) price[1] and
price [1] < (>) price [2]. If either of these two was true, this is an indication of
momentum, therefore triggering an order.
