FGridMsg msg; 
DbFGrid f;
me.dir() + "../samples/threeLayers.fgrd" => string file;
f.open(file);

while(true)
{
    if(f.read(msg) == 0)
    {
        if(msg.type == 0)
            <<< "wait", msg.value >>>;
        else
        if(msg.type == 1)
            <<< "down", msg.note, msg.value >>>;
        else
        if(msg.type == 2)
            <<< "up", msg.note >>>;
    }
    else
        break;
}
