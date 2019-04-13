import ryu_ofctl

# part 1.4 blocking traffic
# blocking all flows from h1 to h3

print "Blocking traffic form h1 ==> h3"

# get flow & set parameter
flow = ryu_ofctl.FlowEntry()
dpid = 1

# get flow from h1 to h2 & h3
act3 = ryu_ofctl.OutputAction(3)
act3 = ryu_ofctl.OutputAction(2)
flow.in_port = 1
flow.addAction(act2)
flow.addAction(act3)
ryu_ofctl.insertFlow(dpid, flow)

# delete flow from h1 to h3
flow.in_port = 1
ryu_ofctl.deleteFlow(dpid, flow)

# delete flow from h3 to h1
flow.in_port = 3
ryu_ofctl.deleteFlow(dpid, flow)

# prevent flow transfers through other hosts
ryu_ofctl.deleteAllFlows(dpid)

# insert flow
ryu_ofctl.insertFlow(dpid, flow)

print "Done blocking all traffic from h1 ==> h3"