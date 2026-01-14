'use client'

import { useState, useEffect } from 'react'

interface MarketData {
  market_id: string
  market: number
  event_name: string
  best_bid: number
  best_ask: number
  bid_size: number
  ask_size: number
}

interface TradeModalProps {
  market: MarketData | null
  isOpen: boolean
  onClose: () => void
  balance: number
  onTrade: (marketId: string, side: 'buy' | 'sell', amount: number, price: number) => void
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function TradeModal({ market, isOpen, onClose, balance, onTrade }: TradeModalProps) {
  const [side, setSide] = useState<'buy' | 'sell'>('buy')
  const [amount, setAmount] = useState<string>('')
  const [estimatedCost, setEstimatedCost] = useState<number>(0)

  useEffect(() => {
    if (isOpen) {
      document.body.style.overflow = 'hidden'
    } else {
      document.body.style.overflow = 'unset'
    }
    return () => {
      document.body.style.overflow = 'unset'
    }
  }, [isOpen])

  useEffect(() => {
    if (market && amount) {
      const numAmount = parseFloat(amount)
      if (!isNaN(numAmount) && numAmount > 0) {
        const price = side === 'buy' ? market.best_ask : market.best_bid
        if (price > 0) {
          setEstimatedCost(numAmount * price)
        } else {
          setEstimatedCost(0)
        }
      } else {
        setEstimatedCost(0)
      }
    } else {
      setEstimatedCost(0)
    }
  }, [amount, side, market])

  const handleTrade = () => {
    if (!market || !amount) return
    
    const numAmount = parseFloat(amount)
    if (isNaN(numAmount) || numAmount <= 0) return

    const price = side === 'buy' ? market.best_ask : market.best_bid
    if (price <= 0) {
      alert('No available price for this side')
      return
    }

    if (side === 'buy' && estimatedCost > balance) {
      alert(`Insufficient balance. You need $${estimatedCost.toFixed(2)} but have $${balance.toFixed(2)}`)
      return
    }

    onTrade(market.market_id, side, numAmount, price)
    setAmount('')
    onClose()
  }

  if (!isOpen || !market) return null

  const currentPrice = side === 'buy' ? market.best_ask : market.best_bid
  const hasPrice = currentPrice > 0

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content trade-modal" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h2 className="modal-title">Place Order</h2>
          <button className="modal-close" onClick={onClose}>×</button>
        </div>

        <div className="modal-body">
          {/* Market Info */}
          <div className="trade-market-info">
            <div className="trade-market-name">{market.event_name}</div>
            <div className="trade-market-subtitle">
              {marketNames[market.market] || `Market ${market.market}`} • {market.market_id.substring(0, 16)}...
            </div>
          </div>

          {/* Balance */}
          <div className="trade-balance">
            <span className="trade-balance-label">Available Balance:</span>
            <span className="trade-balance-value">${balance.toFixed(2)}</span>
          </div>

          {/* Side Selection */}
          <div className="trade-side-selector">
            <button
              className={`trade-side-button ${side === 'buy' ? 'active buy' : ''}`}
              onClick={() => setSide('buy')}
            >
              BUY
            </button>
            <button
              className={`trade-side-button ${side === 'sell' ? 'active sell' : ''}`}
              onClick={() => setSide('sell')}
            >
              SELL
            </button>
          </div>

          {/* Price Display */}
          <div className="trade-price-display">
            <span className="trade-price-label">Price:</span>
            <span className={`trade-price-value ${side === 'buy' ? 'ask' : 'bid'}`}>
              {hasPrice ? (currentPrice * 100).toFixed(2) + '¢' : 'N/A'}
            </span>
            {!hasPrice && (
              <span className="trade-price-warning">No {side === 'buy' ? 'ask' : 'bid'} available</span>
            )}
          </div>

          {/* Amount Input */}
          <div className="trade-input-group">
            <label className="trade-input-label">Amount ($)</label>
            <input
              type="number"
              className="trade-input"
              placeholder="0.00"
              value={amount}
              onChange={(e) => setAmount(e.target.value)}
              min="0"
              step="0.01"
            />
          </div>

          {/* Estimated Cost */}
          {amount && estimatedCost > 0 && (
            <div className="trade-estimate">
              <span className="trade-estimate-label">Estimated Cost:</span>
              <span className="trade-estimate-value">${estimatedCost.toFixed(2)}</span>
            </div>
          )}

          {/* Quick Amount Buttons */}
          <div className="trade-quick-amounts">
            <button 
              className="trade-quick-button"
              onClick={() => setAmount((balance * 0.25).toFixed(2))}
            >
              25%
            </button>
            <button 
              className="trade-quick-button"
              onClick={() => setAmount((balance * 0.5).toFixed(2))}
            >
              50%
            </button>
            <button 
              className="trade-quick-button"
              onClick={() => setAmount((balance * 0.75).toFixed(2))}
            >
              75%
            </button>
            <button 
              className="trade-quick-button"
              onClick={() => setAmount(balance.toFixed(2))}
            >
              MAX
            </button>
          </div>
        </div>

        <div className="modal-footer">
          <button className="modal-button secondary" onClick={onClose}>Cancel</button>
          <button 
            className={`modal-button primary ${side === 'buy' ? 'buy' : 'sell'}`}
            onClick={handleTrade}
            disabled={!hasPrice || !amount || parseFloat(amount) <= 0 || (side === 'buy' && estimatedCost > balance)}
          >
            {side === 'buy' ? 'BUY' : 'SELL'}
          </button>
        </div>
      </div>
    </div>
  )
}

