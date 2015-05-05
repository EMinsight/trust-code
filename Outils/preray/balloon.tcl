# Tcl Library to provide balloon help.
# balloon help bindings are automatically added to all buttons and
# menus by init_balloon call.
# you have only to provide descriptions for buttons or menu items by
# setting elements of global help_tips array, indexed by button path or
# menu path,item to something useful.
# if you want to have balloon helps for any other widget you can
# do so by enable_balloon widget_path_or_class
# or enable_balloon_selective widget_path_or_class Tcl_Script
#
# You can toggle balloon help globally on and off by setting variable
# use_balloons to true or false

proc enable_balloon {name_to_bind {script {}}} {
if ![llength $script] {
bind $name_to_bind <Any-Enter> "+schedule_balloon %W %X %Y"
bind $name_to_bind <Any-Motion> "+reset_balloon %W %X %Y"
} else {
bind $name_to_bind <Any-Enter> "+schedule_balloon %W %X %Y \[$script\]"
bind $name_to_bind <Any-Motion> "+reset_balloon %W %X %Y \[$script\]"
}
bind $name_to_bind <Any-Leave> "+cancel_balloon"
}

proc schedule_balloon {window x y {item {}}} {
global use_balloons help_tips balloon_after_ID 
if !$use_balloons return
if [string length $item] {
set index "$window,$item"
} else {set index $window}
if [info exists help_tips($index)] {
set balloon_after_ID [after $help_tips(delay) "create_balloon \"$help_tips($index)\" $x $y"]
}
}
proc reset_balloon {window x y {item {}}} {
cancel_balloon
schedule_balloon $window $x $y $item
}

proc cancel_balloon {} {
global balloon_after_ID
if [info exists balloon_after_ID] {
after cancel $balloon_after_ID
unset balloon_after_ID 
} else { 
 if [winfo exists .balloon_help] {destroy .balloon_help}
}
}

proc create_balloon {text x y} {
global balloon_after_ID help_tips font
toplevel .balloon_help -relief flat
unset balloon_after_ID
wm overrideredirect .balloon_help true
wm positionfrom .balloon_help program
wm geometry .balloon_help "+[expr $x+5]+[expr $y+5]"
    label .balloon_help.tip -text $text -wraplength $help_tips(width) -bg $help_tips(color) ;#-font $font
pack .balloon_help.tip
}

proc init_balloons {args} {
global help_tips use_balloons
set help_tips(width) 150
set help_tips(color) #ffff60
set help_tips(delay) 1000
getopt help_tips $args
set use_balloons 1
enable_balloon Button
enable_balloon Menubutton
enable_balloon Menu "%W index active" 
}
package provide balloon 1.0

