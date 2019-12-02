
from tensorflow.examples.tutorials.mnist import input_data
import tensorflow as tf
import math

def weight_variable(shape):
  initial = tf.truncated_normal(shape, stddev=1)
  return tf.Variable(initial)

mnist = input_data.read_data_sets("MNIST_data/", one_hot=True)

sess = tf.InteractiveSession()

x = tf.placeholder(tf.float32, [None, 784])
w0 = weight_variable([784, 100])
w1 = weight_variable([100, 50])
w2 = weight_variable([50, 10])

z0 = tf.matmul(x, w0)
hidden0 = tf.nn.sigmoid(z0)

z1 = tf.matmul(hidden0, w1)
hidden1 = tf.nn.sigmoid(z1)

z2 = tf.matmul(hidden1, w2)
y = tf.nn.sigmoid(z2)


# Training
y_ = tf.placeholder(tf.float32, [None, 10])
cross_entropy = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(labels=y_, logits=y))
train_step = tf.train.AdamOptimizer(1e-4).minimize(cross_entropy)
#train_step = tf.train.GradientDescentOptimizer(0.05).minimize(cross_entropy)

'''
for _ in range(20000):
  batch_xs, batch_ys = mnist.train.next_batch(10)
  sess.run(train_step, feed_dict={x: batch_xs, y_: batch_ys})
'''



# Evaluation
correct_prediction = tf.equal(tf.argmax(y,1), tf.argmax(y_,1))
accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

sess.run(tf.global_variables_initializer())

for i in range(20000):
	batch = mnist.train.next_batch(5)
	if i%100 == 0:
		train_accuracy = accuracy.eval(feed_dict={x:batch[0], y_: batch[1]})
		print("step %d, training accuracy %g"%(i, train_accuracy))
	train_step.run(feed_dict={x: batch[0], y_: batch[1]})

print("test accuracy %g"%accuracy.eval(feed_dict={
	x: mnist.test.images, y_: mnist.test.labels}))


'''
print(sess.run(accuracy, feed_dict={x: mnist.test.images, y_: mnist.test.labels}))
'''
