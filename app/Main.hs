module Main where

import Parser
import Text.Megaparsec (parseTest)

main :: IO ()
main = parseTest expr "x+2+2+1"
