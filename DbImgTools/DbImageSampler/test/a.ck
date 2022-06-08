DbImageSampler ismp;
me.dir() + "rgb.png" => ismp.loadImage;
<<<"data unavailable", ismp.getSample(.25, .25) >>>;
20::ms => now; // load
<<<"data available", ismp.getSample(.25, .25) >>>;

vec4 isamp;
ismp.getSample(.25, .25) => isamp;
<<< "UL (red)", isamp >>>;

ismp.getSample(.75, .25) => isamp;
<<< "UR (green)", isamp >>>;

ismp.getSample(.25, .75) => isamp;
<<< "LL (blue)", isamp >>>;

ismp.getSample(.75, .75) => isamp;
<<< "LR (gray)", isamp >>>;

float t;
while(t < 6.28)
{
    .5 + .25 * Math.sin(t)  => float x;
    .5 + .25 * Math.cos(t) => float y;

    ismp.getSample(x, y) => isamp;
    <<<x, y, "rgba", isamp>>>;

    .01::second => now;
    .25 +=> t;
}
