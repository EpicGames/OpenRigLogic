\begin{algorithm}
\DontPrintSemicolon
\KwIn{Dimensions of sub-matrices $Extents[]$}
\NextIn{Input indices $InInd[]$}
\NextIn{Output indices $OutInd[C]$}
\NextIn{Matrix values $Mat[]$}
\NextIn{Input vector $InVec[]$}
\NextIn{Output vector $OutVec[]$}
\KwOut{Modifies $OutVec[]$ inplace}
$valoff \gets 0$\;
$inoff \gets 0$\;
$outoff \gets 0$\;
\ForEach{$submat \in Extents$}{
  \For{$r \gets 0$ \KwTo $submat.rows - 1$ \KwBy $4$}{
    $sum \gets \{0.0, 0.0, 0.0, 0.0\}$\;
    \For{$c \gets 0$ \KwTo $submat.cols - 1$}{
      $i \gets InInd[inoff + c]$\;
      $input \gets set4(InVec_i)$\;
      $block \gets load4(Mat, valoff)$\;
      $sum \gets sum + block * input$\;
      $valoff \gets valoff + 4$\;
    }
    $outidx \gets load4(OutInd, outoff)$\;
    $scatter4(sum, OutVec, outidx)$\;
    $inoff \gets inoff + submat.cols$\;
  }
  $outoff \gets outoff + submat.rows$\;
}
\caption{{\sc EvalLinV} Evaluate Linear Outputs(Vectorized) - See Appendix A}
\label{algo:EvalLinV}
\end{algorithm}