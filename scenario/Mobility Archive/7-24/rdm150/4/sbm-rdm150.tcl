set RXThresh 3.81429e-08 ; #71m

#===================================
#     Simulation parameters setup
#===================================
set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
#set val(prop)   Propagation/FreeSpace
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(nn)     150                         ;# number of mobilenodes
set val(rp)     SB                      ;# routing protocol
set val(x)      500                      ;# X dimension of topography
set val(y)      500                      ;# Y dimension of topography
set val(stop)   70
set val(rdm)    "rdm150-4"
set val(mvnum)  1
#


#===================================
#  Attaching selected headers    
#===================================

remove-all-packet-headers
add-packet-header Mac LL IP ARP LL SB

#===================================
#        Initialization        
#===================================
#Create a ns simulator
set ns_ [new Simulator   -broadcast on]

#Setup topography object
set topo       [new Topography]
$topo load_flatgrid $val(x) $val(y)
create-god $val(nn)
set god_ [God instance]


#Open the NS trace file
set tracefile [open out.tr w]
$ns_ trace-all $tracefile

#Open the NAM trace file
set namfile [open out.nam w]
$ns_ namtrace-all $namfile
$ns_ namtrace-all-wireless $namfile $val(x) $val(y)
set chan [new $val(chan)];#Create wireless channel

Mac/802_11 set dataRate 512Kbps
Phy/WirelessPhy set RXThresh_ $RXThresh

#===================================
#     Node parameter setup
#===================================
$ns_ node-config -adhocRouting  $val(rp) \
                -llType        $val(ll) \
                -macType       $val(mac) \
                -ifqType       $val(ifq) \
                -ifqLen        $val(ifqlen) \
                -antType       $val(ant) \
                -propType      $val(prop) \
                -phyType       $val(netif) \
                -channel       $chan \
                -topoInstance  $topo \
                -agentTrace    ON \
                -routerTrace   ON \
                -macTrace      OFF \
                -movementTrace OFF \

#===================================
#        Nodes Definition        
#===================================

for {set i 0} {$i<$val(nn)} {incr i} {
 set node_($i) [$ns_ node]
}

for {set i 0} {$i<$val(nn)} {incr i} {
 $node_($i) random-motion 0
 set udp($i) [new Agent/SB]
 $ns_ attach-agent $node_($i) $udp($i)
 #$udp($i) set packetSize_ 1000
}


#set myagent [new Agent/MyAgent]
#$myagent set myvar1_ 2

#$myagent myfunc2 
#$myagent set mvnum_ $val(mvnum)
#$myagent set myfunc2
#$myagent set myvar2_ 3.14
#$myagent set topo_th_ 0.3
#$myagent myfunc 10 20.0


$ns_ at 0.0 "[$node_(0) set ragent_] base-station"
source $val(rdm)



for {set i 0} {$i < $val(nn)} {incr i} {
 $ns_ initial_node_pos $node_($i) 10 
}



for {set i 0} {$i < $val(nn)} {incr i} {
 set my_x [expr $i * 10 + 1]
 set my_y [expr $i * 10 + 1]
# $ns at 3.0 "$node_($i) setdest $my_x $my_y 20"
}




#ここまで25s

proc finish {} {
    global ns_ tracefile namfile 
    $ns_ flush-trace
    close $tracefile
    close $namfile
#    exec nam out.nam &
    puts kokokokokokokokokoko
    exit 0
}

#===================================
#  Reset the nodes   
#===================================

for {set i 0} {$i < $val(nn) } { incr i } {
    $ns_ at $val(stop) "$node_($i) reset"
}

#===================================
#  Simulation start up  
#===================================

$ns_ at $val(stop) "$ns_ nam-end-wireless $val(stop)"
$ns_ at $val(stop) "finish"
$ns_ at $val(stop) "puts \"done\" ; $ns_ halt"
puts "before sim run"
$ns_ run


