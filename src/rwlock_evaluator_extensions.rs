use std::sync::{RwLock, RwLockWriteGuard, TryLockResult};
use tcalc::evaluator::{AngleUnit, CancellationTokenSource, Evaluator, Number};

pub trait RwlockEvaluatorExtensions {
    type T: Number;

    fn cancel_evaluation_and_write(&self, cancellation_token_source: &CancellationTokenSource) -> RwLockWriteGuard<Evaluator<Self::T>>;
}

impl<T> RwlockEvaluatorExtensions for RwLock<Evaluator<T>> where T: Number {
    type T = T;
    
    fn cancel_evaluation_and_write(&self, cancellation_token_source: &CancellationTokenSource) -> RwLockWriteGuard<Evaluator<Self::T>> {
        match self.try_write() {
            Ok(locked) => {
                locked
            }
            Err(e) => {
                cancellation_token_source.cancel();
                self.write().unwrap()
            }
        }
    }
}