﻿using System;
using System.Windows.Input;

namespace OCC.Commands
{
    public class RelayCommand<T> : ICommand
    {
        private readonly Action<T> _execute; // 실행 로직
        private readonly Func<T, bool> _canExecute; // 실행 가능 조건

        public RelayCommand(Action<T> execute, Func<T, bool> canExecute = null)
        {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter)
        {
            return _canExecute == null || _canExecute((T)parameter);
        }

        public void Execute(object parameter)
        {
            _execute((T)parameter);
        }

        public event EventHandler CanExecuteChanged
        {
            add => CommandManager.RequerySuggested += value;
            remove => CommandManager.RequerySuggested -= value;
        }
    }
}