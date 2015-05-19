FoldbackSaturator
==

What is it?
--
A foldback saturator is a distortion which "folds back" a signal as it passes a (positive and negative) threshold. This implementation also includes an index, which the signal is multiplied by as it passes the threshold, for a more intense effect.

Methods
--

    makeupGain()

    The amount of makeup gain applied to the signal after processing,
    multiplied against the reciprocal of the threshold. Defaults to 1.0


    threshold()
    
    The threshold (positive and negative) that the signal is inverted
    against as it is passed. Defaults to 0.6
    

    index()

    The index that the signal is multiplied by after it is inverted
    against the threshold. Defaults to 2.0
