# itd-ctl
Reference taskset to do secondary development for ITD feature 

sudo apt install git                                                                                                                                                 
sudo apt install gcc-multilib                                                                                                                                          
sudo apt install libc6-dev 


Usage:
         itd-ctl [options] [mask| cpu-list| group] [cmd] [args ...]


Options:
        -a, --all               display all the active configuration
        -v, --version           display the version and author
        -e, --enable            1 --itd function enable; 0 --itd function disable
        -s, --status            display the pid or process name configuration
        -S, --all status        display all the process configuration
        -c, --cpu-list          display and specify cpus in list format
        -g, --group-list        display and specify groups in list format
        -p, --pid               give pid or process name
        -r, --remove            remove the core configuration for a specify pid or process name
        -h, --hfi table         get hardware feedback interface table and state


Show the help and display all the active configuration
        itd-ctl -a

Show the version of tool
        itd-ctl -v

Enable or disable ITD function
        itd-ctl -e 1 or 0

Bond running process with a special CPU list:
        itd-ctl -c 0,2,4-6 -p firefox OR itd-ctl -c 0,2,4-6 -p 3299

Bond running process with a special Group list reference HFI table
        itd-ctl -g 1-2 -p firefox OR itd-ctl -g 1-2 -p 3299

Query running process's bonding CPU list:
        itd-ctl -s firefox OR itd-ctl -s 3299

Query all the running process's bonding CPU list:
        itd-ctl -S

Remove the core configuration for a specify pid or process name
        itd-ctl -r 3299 OR itd-ctl -r firefox

Get hardware feedback interface table and state
        itd-ctl -h


