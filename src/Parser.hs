{-# LANGUAGE OverloadedStrings #-}

module Parser where

import Text.Megaparsec
import Text.Megaparsec.Char
import Data.Void

type Parser = Parsec Void String
        -- Engine: Error Type (Input Type)
data Expr
    = Var String
    | Number Int
    | Add Expr Expr
    deriving (Show)

expr :: Parser Expr
expr = do
    a <- term
    _ <- char '+'
    b <- term
    return (Add a b)

term :: Parser Expr
term = number <|> variable

number :: Parser Expr
number = Number . read <$> some digitChar

variable :: Parser Expr
variable = Var <$> some letterChar