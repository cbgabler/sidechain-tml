from sklearn.ensemble import RandomForestClassifier
from sklearn.datasets import make_classification
import pandas as pd

CLASSES = ["safe", "timing_leak", "power_leak"]

file = pd.read_json('./data/mock.json')

file.head()