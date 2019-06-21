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
set val(nn)     18                         ;# number of mobilenodes
set val(rp)     SB                      ;# routing protocol
set val(x)      500                      ;# X dimension of topography
set val(y)      500                      ;# Y dimension of topography
set val(stop)   70

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
set ns [new Simulator   -broadcast on]

#Setup topography object
set topo       [new Topography]
$topo load_flatgrid $val(x) $val(y)
create-god $val(nn)


#Open the NS trace file
set tracefile [open out.tr w]
$ns trace-all $tracefile

#Open the NAM trace file
set namfile [open out.nam w]
$ns namtrace-all $namfile
$ns namtrace-all-wireless $namfile $val(x) $val(y)
set chan [new $val(chan)];#Create wireless channel

Mac/802_11 set dataRate 512Kbps
Phy/WirelessPhy set RXThresh_ $RXThresh

#===================================
#     Node parameter setup
#===================================
$ns node-config -adhocRouting  $val(rp) \
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
 set node_($i) [$ns node]
}

for {set i 0} {$i<$val(nn)} {incr i} {
 $node_($i) random-motion 0
 set udp($i) [new Agent/SB]
 $ns attach-agent $node_($i) $udp($i)
 #$udp($i) set packetSize_ 1000
}


#set myagent [new Agent/MyAgent]
#$myagent set myvar1_ 2

#$myagent myfunc2
#$myagent set myfunc2
#$myagent set myvar2_ 3.14
#$myagent set topo_th_ 0.3
#$myagent myfunc 10 20.0


$ns at 0.0 "[$node_(0) set ragent_] base-station"
$node_(0) set X_ 0
$node_(0) set Y_ 175
$node_(0) set Z_ 0

$node_(1) set X_ 50
$node_(1) set Y_ 200
$node_(1) set Z_ 0

$node_(2) set X_ 50
$node_(2) set Y_ 150
$node_(2) set Z_ 0

$node_(3) set X_ 100
$node_(3) set Y_ 250
$node_(3) set Z_ 0

$node_(4) set X_ 100
$node_(4) set Y_ 200
$node_(4) set Z_ 0

$node_(5) set X_ 100
$node_(5) set Y_ 150
$node_(5) set Z_ 0

$node_(6) set X_ 100
$node_(6) set Y_ 100
$node_(6) set Z_ 0

$node_(7) set X_ 150
$node_(7) set Y_ 250
$node_(7) set Z_ 0

$node_(8) set X_ 150
$node_(8) set Y_ 200
$node_(8) set Z_ 0

$node_(9) set X_ 150
$node_(9) set Y_ 150
$node_(9) set Z_ 0

$node_(10) set X_ 150
$node_(10) set Y_ 100
$node_(10) set Z_ 0

$node_(11) set X_ 200
$node_(11) set Y_ 250
$node_(11) set Z_ 0

$node_(12) set X_ 200
$node_(12) set Y_ 200
$node_(12) set Z_ 0

$node_(13) set X_ 200
$node_(13) set Y_ 150
$node_(13) set Z_ 0

$node_(14) set X_ 200
$node_(14) set Y_ 100
$node_(14) set Z_ 0

$node_(15) set X_ 250
$node_(15) set Y_ 200
$node_(15) set Z_ 0

$node_(16) set X_ 250
$node_(16) set Y_ 150
$node_(16) set Z_ 0

$node_(17) set X_ 300
$node_(17) set Y_ 170
$node_(17) set Z_ 0



for {set i 0} {$i < $val(nn)} {incr i} {
 $ns initial_node_pos $node_($i) 10 
}



for {set i 0} {$i < $val(nn)} {incr i} {
 set my_x [expr $i * 10 + 1]
 set my_y [expr $i * 10 + 1]
# $ns at 3.0 "$node_($i) setdest $my_x $my_y 20"
}


$ns at 2.0 "$node_(7) setdest 150 50 16.6"
$ns at 2.0 "$node_(8) setdest 150 300 16.6"
$ns at 2.0 "$node_(9) setdest 150 50 16.6"
$ns at 2.0 "$node_(10) setdest 150 300 16.6"

$ns at 8.0 "$node_(8) setdest 150 50 16.6"
$ns at 8.0 "$node_(9) setdest 150 300 16.6"

$ns at 10.0 "$node_(7) setdest 150 300 16.6"
$ns at 10.0 "$node_(10) setdest 150 50 16.6"

$ns at 23.0 "$node_(8) setdest 150 300 16.6"
$ns at 23.0 "$node_(9) setdest 150 50 16.6"

$ns at 25.0 "$node_(7) setdest 150 50 16.6"
$ns at 25.0 "$node_(10) setdest 150 300 16.6"

$ns at 38.0 "$node_(8) setdest 150 50 16.6"
$ns at 38.0 "$node_(9) setdest 150 300 16.6"

$ns at 40.0 "$node_(7) setdest 150 300 16.6"
$ns at 40.0 "$node_(10) setdest 150 50 16.6"

$ns at 53.0 "$node_(8) setdest 150 300 16.6"
$ns at 53.0 "$node_(9) setdest 150 50 16.6"

$ns at 55.0 "$node_(7) setdest 150 50 16.6"
$ns at 55.0 "$node_(10) setdest 150 300 16.6"

$ns at 68.0 "$node_(8) setdest 150 50 16.6"
$ns at 68.0 "$node_(9) setdest 150 300 16.6"

$ns at 70.0 "$node_(7) setdest 150 300 16.6"
$ns at 70.0 "$node_(10) setdest 150 50 16.6"




$ns at 2.0 "$node_(11) setdest 200 300 16.6"
$ns at 2.0 "$node_(12) setdest 200 50 16.6"
$ns at 2.0 "$node_(13) setdest 200 300 16.6"
$ns at 2.0 "$node_(14) setdest 200 50 16.6"

$ns at 5.0 "$node_(11) setdest 200 50 16.6"
$ns at 5.0 "$node_(14) setdest 200 300 16.6"

$ns at 11.0 "$node_(12) setdest 200 300 16.6"
$ns at 11.0 "$node_(13) setdest 200 50 16.6"


$ns at 20.0 "$node_(11) setdest 200 300 16.6"
$ns at 20.0 "$node_(14) setdest 200 50 16.6"

$ns at 26.0 "$node_(12) setdest 200 50 16.6"
$ns at 26.0 "$node_(13) setdest 200 300 16.6"

$ns at 35.0 "$node_(11) setdest 200 50 16.6"
$ns at 35.0 "$node_(14) setdest 200 300 16.6"

$ns at 41.0 "$node_(12) setdest 200 300 16.6"
$ns at 41.0 "$node_(13) setdest 200 50 16.6"

$ns at 50.0 "$node_(11) setdest 200 300 16.6"
$ns at 50.0 "$node_(14) setdest 200 50 16.6"

$ns at 56.0 "$node_(12) setdest 200 50 16.6"
$ns at 56.0 "$node_(13) setdest 200 300 16.6"

$ns at 66.0 "$node_(11) setdest 200 50 16.6"
$ns at 66.0 "$node_(14) setdest 200 300 16.6"

#$ns at 72.0 "$node_(12) setdest 200 300 16.6"
#$ns at 72.0 "$node_(13) setdest 200 50 16.6"

#ここまで25s

proc finish {} {
    global ns tracefile namfile 
    $ns flush-trace
    close $tracefile
    close $namfile
#    exec nam out.nam &
    exit 0
}

#===================================
#  Reset the nodes   
#===================================

for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(stop) "$node_($i) reset"
}

#===================================
#  Simulation start up  
#===================================

$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "finish"
$ns at $val(stop) "puts \"done\" ; $ns halt"
puts "before sim run"
$ns run


