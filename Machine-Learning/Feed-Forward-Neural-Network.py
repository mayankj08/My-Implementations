#
# This is implementation of 
# feed forward neural network 
# using tensorflow on MNIST dataset.
#
# Few of the parameters which can be tweaked are:
#  layer_nodes = nodes in each layer.
#  layer_nodes[0] should be count of input features
#  layer_nodes[-1] should be out or number of classes
#  layer_nodes[1:-1] are nodes in hidden layers
#
#  learning_rate : learning rate for optimizer
#  epochs : Number of epochs/iterations to run
#  batch_size : Size of batch per run 

import tensorflow as tf
from tensorflow.examples.tutorials.mnist import input_data

mnist = input_data.read_data_sets("/tmp/data/", one_hot=True)

################################
Tweak below parameters to change
behaviour of the neural netowrk
################################
layer_nodes = [784,256,256,10]
learning_rate = 0.001
epochs = 50
batch_size = 100
################################

n_input = layer_nodes[0]
n_classes = layer_nodes[-1]

x = tf.placeholder("float",[None,n_input])
y = tf.placeholder("float",[None,n_classes])

weights = list()
biases = list()
last_node_size = n_input

for i in range(1,len(layer_nodes)):
    curr_weight = tf.Variable(tf.random_normal([last_node_size,layer_nodes[i]]))
    curr_bias = tf.Variable(tf.random_normal([layer_nodes[i]]))
    weights.append(curr_weight)
    biases.append(curr_bias)
    last_node_size = layer_nodes[i]


def multilayer_perceptron():
    # To perform deep copy
    # I don't want to change x. So just copying it
    last_layer_out = tf.identity(x)   

    # performing wx+b till output layer
    for i in range(1,len(layer_nodes)-1):
        last_layer_out = tf.matmul(last_layer_out,weights[i-1])
        last_layer_out = tf.add(last_layer_out,biases[i-1])
        last_layer_out = tf.nn.relu(last_layer_out)
    pred = tf.add(tf.matmul(last_layer_out,weights[-1]),biases[-1])
    
    return pred;

pred = multilayer_perceptron()

# Now let's backpropogate
cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=pred,labels=y))
optimizer = tf.train.AdamOptimizer(learning_rate = learning_rate).minimize(cost)

init = tf.global_variables_initializer()

with tf.Session() as sess:
    sess.run(init);

    for epoch in range(epochs):

        avg_cost = 0
        number_of_batches = int(mnist.train.num_examples/batch_size)
        
        # Loop over all the batches
        for i in range(number_of_batches):
            batch_x, batch_y = mnist.train.next_batch(batch_size)
            _, c = sess.run([optimizer, cost], feed_dict={x: batch_x,y: batch_y})
            avg_cost += c / number_of_batches
        
        print("Epoch:", '%04d' % (epoch+1), "cost=", "{:.9f}".format(avg_cost))
    print("Completed Optimization!!")

    # Time to test the model
    correct_prediction = tf.equal(tf.argmax(pred, 1), tf.argmax(y, 1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, "float"))
    print("Accuracy:", accuracy.eval({x: mnist.test.images, y: mnist.test.labels})) 
