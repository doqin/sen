{-# LANGUAGE OverloadedStrings #-}

module Parser where

import Text.Megaparsec
import Text.Megaparsec.Char
import Control.Monad.Combinators.Expr
import Data.Void

type Parser = Parsec Void String
        -- Engine: Error Type (Input Type)
data Expr
    = Var String
    | Number Int
    | Add Expr Expr
    deriving (Show)

expr :: Parser Expr -- is a parser that parses an Expr from a string input
expr = makeExprParser term operatorTable
  where
    operatorTable :: [[Operator Parser Expr]]
    operatorTable = 
      [ [ binary "+" Add ] ]

binary :: String -> (Expr -> Expr -> Expr) -> Operator Parser Expr
binary name f = InfixL (f <$ symbol name)

symbol :: String -> Parser String
symbol s = string s <* space

term :: Parser Expr
term = number <|> variable <|> parens expr

parens :: Parser a -> Parser a
parens = between (symbol "(") (symbol ")")

number :: Parser Expr
number = Number . read <$> some digitChar <* space

variable :: Parser Expr
variable = Var <$> some letterChar <* space